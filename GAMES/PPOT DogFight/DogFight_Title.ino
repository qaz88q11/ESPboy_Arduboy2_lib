#include "src/utils/Arduboy2Ext.h"

#include <FixedPoints.h>
#include <FixedPointsCommon.h>
#include "src/images/Images.h"

// y2 = y1+2 always, so only store y1 (4 bytes vs 8)
static const uint8_t PROGMEM exhaust[4] = { 5, 11, 6, 10 };

int16_t title_X[2];
uint8_t title_Y[2];


// ------------------------------------------------------------------

void title_Init() {

    title_X[0] = -32;
    title_X[1] = -100;
    title_Y[0] = 0;
    title_Y[1] = 0;

    arduboy.initRandomLFSRSeed();
    gameState = GameState::Title;
    arduboy.frameCount = 0;
    arduboy.setFrameRate(60);

}

// ------------------------------------------------------------------

void title_Display() {

    if (arduboy.justPressed(A_BUTTON) || arduboy.justPressed(B_BUTTON)) {

        arduboy.setFrameRate(30);
        initEnemies(arduboy, enemies, Constants::Enemy_Count);

        gameState = GameState::Playing;
        speedFactor = Constants::SpeedFactor_Init;
        player.reset(arduboy.justPressed(B_BUTTON));
        initHeart();

    }

    Sprites::drawOverwrite(8,   20, Images::Letter_D, 0);
    Sprites::drawOverwrite(22,  19, Images::Letter_O, 0);
    Sprites::drawOverwrite(37,  19, Images::Letter_G, 0);
    Sprites::drawOverwrite(57,  20, Images::Letter_F, 0);
    Sprites::drawOverwrite(71,  20, Images::Letter_I, 0);
    Sprites::drawOverwrite(76,  19, Images::Letter_G, 0);
    Sprites::drawOverwrite(92,  20, Images::Letter_H, 0);
    Sprites::drawOverwrite(107, 20, Images::Letter_T, 0);

    Sprites::drawOverwrite(7, 1, Images::Title_Clouds_Top, 0);
    Sprites::drawOverwrite(7, 43, Images::Title_Clouds_Top, 0);

    if (arduboy.frameCount > 64 && arduboy.frameCount % 64 < 32) Sprites::drawOverwrite(33, 58, Images::PressA, 0);

    for (uint8_t i = 0; i < 2; i++) {

        title_X[i] = title_X[i] + 1;

        // Compute bobbing offset via fpSin — replaces 142-byte PROGMEM table.
        int16_t adjusted = title_X[i] + 32;
        uint16_t phase   = (uint16_t)(((adjusted % 142) + 142) % 142);
        uint16_t deg     = phase * 360 / 142;
        title_Y[i]       = 20 + (int8_t)(fpSin(deg) * FP8(8));

        Sprites::drawExternalMask(title_X[i], title_Y[i], Images::Title_Plane, Images::Title_Plane_Mask, 0, 0);

        uint8_t c  = arduboy.frameCount % 4;
        uint8_t y1 = pgm_read_byte(&exhaust[c]);
        uint8_t y2 = y1 + 2;
        arduboy.drawPixel(title_X[i] + 32, title_Y[i] + y1, WHITE);
        arduboy.drawPixel(title_X[i] + 32, title_Y[i] + y2, WHITE);

        // Reset position if off screen ..

        if (title_X[i] > 150) {
            title_X[i] = -32;
        }

    }

}
