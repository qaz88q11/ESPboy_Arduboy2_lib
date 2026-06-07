#include "src/utils/Arduboy2Ext.h"
#include "src/utils/Constants.h"
#include <FixedPoints.h>
#include <FixedPointsCommon.h>
#include "src/images/Images.h"

// ── Drop animation ────────────────────────────────────────────────────────

static const uint8_t DROP_X[Constants::Drop_Letter_Count] PROGMEM = {
    3, 18, 33, 49, 65, 80, 95, 109
};

static int16_t  letY2[Constants::Drop_Letter_Count];
static int16_t  letV2[Constants::Drop_Letter_Count];
static bool     letActive[Constants::Drop_Letter_Count];
static bool     letSettled[Constants::Drop_Letter_Count];
static uint16_t dropTimer;

// ------------------------------------------------------------------

void gameOver_Init() {

    for (uint8_t i = 0; i < Constants::Drop_Letter_Count; i++) {
        letY2[i]      = Constants::Drop_Start_Y2;
        letV2[i]      = 0;
        letActive[i]  = false;
        letSettled[i] = false;
    }

    dropTimer = 0;
    arduboy.invert(false);
    gameState = GameState::GameOver_Drop;

}

// ------------------------------------------------------------------

void gameOver_Drop() {

    dropTimer++;

    render(player);

    for (uint8_t i = 0; i < Constants::Drop_Letter_Count; i++) {
        if (!letActive[i] && dropTimer >= (uint16_t)(i * Constants::Drop_Stagger + 1)) {
            letActive[i] = true;
        }
    }

    bool allSettled = true;

    for (uint8_t i = 0; i < Constants::Drop_Letter_Count; i++) {

        if (!letActive[i]) { allSettled = false; continue; }

        if (!letSettled[i]) {

            letV2[i] += Constants::Drop_Gravity;
            letY2[i] += letV2[i];

            if (letY2[i] >= Constants::Drop_Ground_Y2) {
                letY2[i] = Constants::Drop_Ground_Y2;
                letV2[i] = -(letV2[i] * Constants::Drop_Restitution_N / Constants::Drop_Restitution_D);
                if (letV2[i] > -Constants::Drop_Settle_Vel) {
                    letV2[i]      = 0;
                    letSettled[i] = true;
                }
            }

        }

        int8_t  px   = (int8_t)pgm_read_byte(&DROP_X[i]);
        int16_t py   = letY2[i] >> 1;

        const uint8_t* img  = (const uint8_t*)pgm_read_dword(&Images::GameOver_Images[i]);
        const uint8_t* mask = (const uint8_t*)pgm_read_dword(&Images::GameOver_Images_Mask[i]);

        Sprites::drawExternalMask(px, (int8_t)py, img, mask, 0, 0);

        if (!letSettled[i]) allSettled = false;

    }

    if (allSettled) {
        gameState = GameState::GameOver;
        arduboy.frameCount = 0;
    }

}

// ------------------------------------------------------------------

void gameOver_Display() {

    if (arduboy.justPressed(A_BUTTON)) {
        gameState = GameState::Title_Init;
    }

    render(player);

    for (uint8_t i = 0; i < Constants::Drop_Letter_Count; i++) {
        
        int8_t         px   = (int8_t)pgm_read_byte(&DROP_X[i]);
        
        const uint8_t* img  = (const uint8_t*)pgm_read_dword(&Images::GameOver_Images[i]);
        const uint8_t* mask = (const uint8_t*)pgm_read_dword(&Images::GameOver_Images_Mask[i]);

        Sprites::drawExternalMask(px, Constants::Drop_Ground_Y2 >> 1, img, mask, 0, 0);
    }

    if (arduboy.frameCount > 96) {
    
        if (arduboy.frameCount % 64 < 32) {

            Sprites::drawOverwrite(33, 58, Images::PressA, 0);

        }
        else {
    
            uint16_t score   = player.getScore();
            uint8_t hundreds = score / 100;
            uint8_t tens     = (score / 10) % 10;
            uint8_t units    = score % 10;

            Sprites::drawOverwrite(Constants::Screen_Half_W - 19, 58, Images::GameOver_Score, 0);
            Sprites::drawOverwrite(Constants::Screen_Half_W + 6, 59, Images::Score_Nums, hundreds);
            Sprites::drawOverwrite(Constants::Screen_Half_W + 10, 59, Images::Score_Nums, tens);
            Sprites::drawOverwrite(Constants::Screen_Half_W + 14, 59, Images::Score_Nums, units);

        }    
    
    }

}
