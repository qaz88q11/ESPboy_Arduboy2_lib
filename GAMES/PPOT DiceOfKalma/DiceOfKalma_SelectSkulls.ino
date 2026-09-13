#include <ArduboyFX.h>  
#include "src/utils/Constants.h" 


// Offer three unique skulls ..

void offerSkulls() {

    uint8_t skullsBot[] = { 0, 1, 1, 2, 2, 3, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 19 };
    uint8_t skullsTop[] = { 2, 3, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 18, 19, 19, 19 };

    uint8_t skullA = random(skullsBot[level - 2], skullsTop[level - 2]);
    uint8_t skullB = 0;

    do {
        skullB = random(skullsBot[level - 2], skullsTop[level - 2]);
    } while (skullB == skullA);

    skullChoiceA = static_cast<SkullType>(SkullTypeOrder[skullA]);
    skullChoiceB = static_cast<SkullType>(SkullTypeOrder[skullB]);

}


void skullChoice_Init() {

    saveHighScore();
    offerSkulls();
    gameState = GameState::Game_Skull_Choice;
    skullCursor = 0;
  
}


void skullChoice() {


    // Handle input ..
        
    if (arduboy.justPressed(RIGHT_BUTTON) && skullCursor > 0) {
        
        skullCursor = skullCursor - 1;

    }
    else if (arduboy.justPressed(LEFT_BUTTON) && skullCursor < 1) {
        
        skullCursor = skullCursor + 1;

    }
    else if (arduboy.justPressed(A_BUTTON)) {

        SkullType chosen;
        
        switch (skullCursor) {

            case 0:
                chosen = skullChoiceA;
                break;

            case 1:
                chosen = skullChoiceB;
                break;
                
        }

        if (hand.getDeckCount() < MAX_DECK) {

            hand.addSkullToDeck(chosen);
            gameState = GameState::Game_Upgrade_Choice_Init;
    
        } 
        else {

            pendingSkull = chosen;
            deckViewCursor = 0;
            gameState = GameState::Game_Deck_Full_Swap;

        }

    }
    else if (arduboy.justPressed(B_BUTTON)) {

        gameState = GameState::Game_Upgrade_Choice_Init;

    }


    // Render screen ..

    FX::drawBitmap(115, 0, Images::SkullSelect_Top, 0, dbmNormal);
    FX::drawBitmap(0, 0, Images::SkullSelect_Bot, 0, dbmNormal);

    uint24_t aIcon = FX::readIndexedUInt24(Images::Skulls, static_cast<uint8_t>(skullChoiceA));
    FX::drawBitmap(64, 0, aIcon, 0, dbmNormal);

    uint24_t bIcon = FX::readIndexedUInt24(Images::Skulls, static_cast<uint8_t>(skullChoiceB));
    FX::drawBitmap(64 - 48, 0, bIcon, 0, dbmNormal);

    if (arduboy.frameCount % 24 < 12) {
        FX::drawBitmap(64 - (skullCursor * 48), 0, Images::Skull_Cursor, 0, dbmWhite);
    }
    
}

void  drawSkullInfo() {

    FX::drawBitmap(112, 0, Images::SkullInfo_Top, 0, dbmNormal);
    FX::drawBitmap(-3, 0, Images::SkullInfo_Bot, 0, dbmNormal);

    uint24_t img = FX::readIndexedUInt24(Images::Skulls, static_cast<uint8_t>(skullInfoType));
    FX::drawBitmap(65, 0, img, 0, dbmNormal);

    if (arduboy.justPressed(A_BUTTON)) {
        gameState = returnState;
    }

}
