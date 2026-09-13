#include <ArduboyFX.h>  

#include "fxdta.h"
#include "fxdata/fxdata.h"
#include "src/utils/Arduboy2Ext.h"
#include "src/utils/Constants.h"
#include "src/utils/Structs.h"
#include "src/utils/Stack.h"
#include "src/entities/Hand.h"
#include "src/entities/Particle.h"
#include "src/entities/SkullData.h"
#include "src/entities/Cookie.h"

#include "images.h"

extern void nbSPI_writeBytes(uint8_t *data, uint16_t size);
extern bool nbSPI_isBusy();
extern ArduboySettings arduboySaveLoadSettings;
extern ESPboyInit myESPboy;

Arduboy2Ext arduboy;

uint32_t *lineBuffer;


uint8_t renderHandResult_Counter = 0;
uint8_t renderHandResult_Timer = 0;
uint8_t countdownDiv = 0;

GameState gameState = GameState::Splash;
GameState returnState = GameState::Splash; // where DECK_VIEW returns to

uint8_t stateTimer = 0;
uint8_t cursor = 0;
uint8_t rollDice_Counter = 0;

// ---------------------------------------------------------------------
// Run state
// ---------------------------------------------------------------------

uint8_t  level = 1;
int16_t threshold = 0;
int16_t thresholdMin = 0;
uint8_t  handsLeft = 0;
uint8_t  handsMax  = 0;
uint8_t  rerollsLeft = 0;
uint8_t  rerollsMax  = 0;
uint16_t bCounter = 0;

Hand hand;

// last hand result, for the result screen

Cookie cookie;
HandScore tempHandScore;
Particle particles[Constants::ParticlesMax];

SkullType skullChoiceA, skullChoiceB;
SkullType skullInfoType;
SkullType pendingSkull = SkullType::None;   

uint8_t skullCursor = 0;
uint8_t upgradeCursor = 0;
uint8_t upgradeTop = 0;
uint8_t deckViewCursor = 0;
uint8_t deckViewTop = 0;
uint8_t messageIdx = 0;
uint16_t takeTooLong = 0;
uint8_t takeTooLong_Type = 0;

Stack <uint16_t, 60> skullStack;
SkullData skullData;

void customDisplay(bool clearBuffer) {
    static uint32_t tickcount = 0;
    uint8_t keys = 0;
    if (millis() - tickcount > 50) {
        keys = myESPboy.getKeys();
        tickcount = millis();
    }
    
    if ((keys & PAD_RGT) || (keys & PAD_LFT)) { 
        while(nbSPI_isBusy()); 
        noInterrupts();
        delay(50);
        keys = myESPboy.getKeys();
        
        if ((keys & PAD_RGT) && (keys & PAD_LFT)) {
            arduboySaveLoadSettings.arduboyYscale = !arduboySaveLoadSettings.arduboyYscale;
            while (myESPboy.getKeys()) delay(10);
            EEPROM.put(EEPROM_STORAGE_SPACE_START_SETTINGS, arduboySaveLoadSettings);
            EEPROM.commit();
            myESPboy.tft.fillScreen(0);
        }
        else if (keys & PAD_RGT) {
            arduboySaveLoadSettings.arduboyBackground++; 
            if (arduboySaveLoadSettings.arduboyBackground > 18)
                arduboySaveLoadSettings.arduboyBackground = 0; 
            while (myESPboy.getKeys()) delay(10);
            EEPROM.put(EEPROM_STORAGE_SPACE_START_SETTINGS, arduboySaveLoadSettings);
            EEPROM.commit();
        }
        else if (keys & PAD_LFT) {
            arduboySaveLoadSettings.arduboyForeground++; 
            if (arduboySaveLoadSettings.arduboyForeground > 18)
                arduboySaveLoadSettings.arduboyForeground = 0;
            while (myESPboy.getKeys()) delay(1);  
            EEPROM.put(EEPROM_STORAGE_SPACE_START_SETTINGS, arduboySaveLoadSettings);
            EEPROM.commit();
        }
        interrupts();
    }
    // -----------------------------------------------------------------------------

    uint16_t foregroundColor = Arduboy2Core::colors[arduboySaveLoadSettings.arduboyForeground];
    uint16_t backgroundColor = Arduboy2Core::colors[arduboySaveLoadSettings.arduboyBackground];
    
    if (Arduboy2Core::invert_flag) {
        uint16_t tmp = foregroundColor;
        foregroundColor = backgroundColor;
        backgroundColor = tmp;
    }

    foregroundColor = (foregroundColor >> 8) | (foregroundColor << 8);
    backgroundColor = (backgroundColor >> 8) | (backgroundColor << 8);

    bool scale = arduboySaveLoadSettings.arduboyYscale;

    while(nbSPI_isBusy()); 

    if (scale) {
        myESPboy.tft.setAddrWindow(0, 0, 128, 128); 
    } else {
        myESPboy.tft.setAddrWindow(32, 0, 64, 128); 
    }

    uint8_t bufIdx = 0;

    for (int16_t chunk = 0; chunk < 16; chunk++) {
        int addr = 0;

        uint16_t *currentBuffer = (uint16_t*)(lineBuffer + (bufIdx * 512));

        for (int16_t line = 0; line < 8; line++) {
            int16_t espY = chunk * 8 + line;
            int16_t logicalX = 127 - espY; 

            for (int16_t espX = 0; espX < 64; espX++) {
                int16_t logicalY = espX;
                
                uint8_t page = logicalY / 8;
                uint8_t bit = logicalY % 8;
                
                bool pixel = Arduboy2Core::sBuffer[logicalX + page * 128] & (1 << bit);
                uint16_t color = pixel ? foregroundColor : backgroundColor;
                
                currentBuffer[addr++] = color;
                if (scale) {
                    currentBuffer[addr++] = color; 
                }
            }
        }

        while(nbSPI_isBusy()); 
        nbSPI_writeBytes((uint8_t*)currentBuffer, addr * 2);
        bufIdx = 1 - bufIdx;
    }
    while(nbSPI_isBusy());

    if (clearBuffer) {
        memset(Arduboy2Core::sBuffer, 0, 128 * 8);
    }
}


void setup() {
    //Serial.begin(115200);
    arduboy.begin();
    arduboy.setFrameRate(30);

    lineBuffer = (uint32_t*)malloc(1024 * sizeof(uint32_t));

    customDisplay(true);
    FX::begin(FX_DATA_PAGE, FX_SAVE_PAGE);
    FX::loadGameState((uint8_t*)&cookie, sizeof(cookie));

}

void loop() {

    if (!arduboy.nextFrame()) return;
    arduboy.pollButtons();
    arduboy.clear();

    switch (gameState) {

        case GameState::Splash:         
            splashScreen();         
            break;

        case GameState::Title:         
            title();         
            break;

        case GameState::Game_Level_Intro:   
            levelIntro();    
            break;

        case GameState::Game_Roll_Dice:          
            renderRollDice();          
            break;

        case GameState::Game_Roll:          
            updateRoll();          
            drawSkull();
            drawLevelAndTarget(hand.getLastHandScore(), level, threshold);

            if (bCounter > 16) {
                drawBonesMultTotal(hand.getLastHandScore());
            }
            else {
                FX::drawBitmap(0, 0, Images::Background_00, 0, dbmNormal);
            }

            drawDice();
            drawFooterRoll();    
            break;

        case GameState::Game_Hand_Result_Init:  
            thresholdMin = (threshold > hand.getLastHandScore().score ? threshold - hand.getLastHandScore().score : 0); 
            renderHandResult_Counter = 0;
            tempHandScore.reset();
            gameState = GameState::Game_Hand_Result_Base;
            [[fallthrough]]

        case GameState::Game_Hand_Result_Base:   
            renderHandResult_Base();           
            break;

        case GameState::Game_Hand_Result_Hand:   
            renderHandResult_Hand();           
            break;

        case GameState::Game_Hand_Result_Skulls_Played:   
            renderHandResult_SkullsPlayed();           
            break;

        case GameState::Game_Hand_Result_Upgrades_Played:   
            renderHandResult_UpgradesPlayed();           
            break;

        case GameState::Game_Hand_Result_Countdown:
            renderHandResult_Countdown();
            break;

        case GameState::Game_Skull_Choice_Init:  
            skullChoice_Init();   
            [[fallthrough]]

        case GameState::Game_Skull_Choice:  
            skullChoice(); 
            break;

        case GameState::Game_Upgrade_Choice_Init:  
            upgradeHand_Init();   
            [[fallthrough]]

        case GameState::Game_Upgrade_Choice:  
            upgradeHand();   
            break;

        case GameState::Game_Skull_Info:  
            drawSkullInfo();   
            break;

        case GameState::Game_Hand_Info_Init:  
            handInfo_Init();   
            [[fallthrough]]

        case GameState::Game_Hand_Info:  
            handInfo();   
            break;

        case GameState::Game_Deck_Full_Swap:
        case GameState::Game_Deck_View:     
            deckView();      
            break;

        case GameState::Game_Over:      
            gameOver();      
            break;

        case GameState::Game_Win:           
            win();           
            break;
    }

    updateAndRenderParticles();
    customDisplay(true);

    if (!skullStack.isEmpty() && arduboy.isFrameCount(2)) {
        uint16_t data = skullStack.pop();
        skullData.setData(data);
    }

}
