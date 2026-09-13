#include <Arduboy2.h>
#include "images.h"


// ---------------------------------------------------------------------
// Drawing
// ---------------------------------------------------------------------

void drawDice() {

    for (uint8_t i = 0; i < 5; i++) {

        switch (hand.getDisplayType(i)) {

            // case DisplayType::Display:
            //     FX::drawBitmap(27, i * 13, Images::Dice_Normal, ((hand.getDice(i) - 1) * 12) + 11, dbmNormal);
            //     break;

            case DisplayType::RollAgain:
                if (rollDice_Counter == 0) {
                    // FX::drawBitmap(27, i * 13, Images::Dice_Normal, ((hand.getDice(i) - 1) * 11) + (arduboy.frameCount / 2) % 11, dbmNormal);
                    FX::drawBitmap(27, i * 13, Images::Dice_Roll, (arduboy.frameCount / 2) % 9, dbmNormal);
                }
                else {
                    FX::drawBitmap(27, i * 13, Images::Dice_Roll, rollDice_Counter - 1, dbmNormal);
                }
                break;

            case DisplayType::Display:
                // FX::drawBitmap(27, i * 13, Images::Dice_Reverse, ((hand.getDice(i) - 1) * 11) + 10, dbmNormal);
                FX::drawBitmap(27, i * 13, Images::Dice_Fade, ((hand.getDice(i) - 1) * 10) + 9, dbmNormal);
                break;

            case DisplayType::Hide:
                FX::drawBitmap(27, i * 13, Images::Dice_Black, hand.getDice(i) - 1, dbmNormal);
                break;

            case DisplayType::Fade:
                FX::drawBitmap(27, i * 13, Images::Dice_Fade, ((hand.getDice(i) - 1) * 10) + renderHandResult_Counter - 1, dbmNormal);
                break;
                
        }

        if (i == cursor && rollDice_Counter == 0 && hand.getDice(i) != 7 && arduboy.frameCount % 24 < 12) {
            FX::drawBitmap(33, (i * 13) - 2, Images::Skeleton_Hand, 0, dbmMasked);
        }

    }

}


void drawNumber_Padded(uint8_t x, int8_t y, uint16_t number, uint8_t digits) {

    switch (digits) {
    
        case 1:
            {
                Sprites::drawSelfMasked(x, y, Images::Numbers_WB, number);

            }
            break;

        case 2:
            {
                Sprites::drawSelfMasked(x, y, Images::Numbers_WB, number / 10);
                Sprites::drawSelfMasked(x, y + 4, Images::Numbers_WB, number % 10);

            }
            break;

        case 3:
            {
                uint16_t thresh = threshold;
                Sprites::drawSelfMasked(x, y, Images::Numbers_WB, number / 100);
                number = number - ((number / 100) * 100);
                Sprites::drawSelfMasked(x, y + 4, Images::Numbers_WB, number / 10);
                Sprites::drawSelfMasked(x, y + 8, Images::Numbers_WB, number % 10);

            }
            break;

        case 4:
            {
                Sprites::drawSelfMasked(x, y, Images::Numbers_WB, number / 1000);
                number = number - ((number / 1000) * 1000);
                Sprites::drawSelfMasked(x, y + 4, Images::Numbers_WB, number / 100);
                number = number - ((number / 100) * 100);
                Sprites::drawSelfMasked(x, y + 8, Images::Numbers_WB, number / 10);
                Sprites::drawSelfMasked(x, y + 12, Images::Numbers_WB, number % 10);

            }
            break;
    
    }

}

void drawNumber_Right(uint8_t x, int8_t y, uint16_t number) {

    switch (number) {
    
        case 0 ... 9:
            {
                Sprites::drawSelfMasked(x, y + 12, Images::Numbers_WB, number);

            }
            break;

        case 10 ... 99:
            {
                Sprites::drawSelfMasked(x, y + 8, Images::Numbers_WB, number / 10);
                Sprites::drawSelfMasked(x, y + 12, Images::Numbers_WB, number % 10);

            }
            break;

        case 100 ... 999:
            {
                uint16_t thresh = threshold;
                Sprites::drawSelfMasked(x, y + 4, Images::Numbers_WB, number / 100);
                number = number - ((number / 100) * 100);
                Sprites::drawSelfMasked(x, y + 8, Images::Numbers_WB, number / 10);
                Sprites::drawSelfMasked(x, y + 12, Images::Numbers_WB, number % 10);

            }
            break;

        case 1000 ... 9999:
            {
                Sprites::drawSelfMasked(x, y, Images::Numbers_WB, number / 1000);
                number = number - ((number / 1000) * 1000);
                Sprites::drawSelfMasked(x, y + 4, Images::Numbers_WB, number / 100);
                number = number - ((number / 100) * 100);
                Sprites::drawSelfMasked(x, y + 8, Images::Numbers_WB, number / 10);
                Sprites::drawSelfMasked(x, y + 12, Images::Numbers_WB, number % 10);

            }
            break;
    
    }

}

void drawNumber(uint8_t x, int8_t y, uint16_t number) {

    switch (number) {
    
        case 0 ... 9:
            {
                Sprites::drawSelfMasked(x, y + 6, Images::Numbers_WB, number);

            }
            break;

        case 10 ... 99:
            {
                Sprites::drawSelfMasked(x, y + 4, Images::Numbers_WB, number / 10);
                Sprites::drawSelfMasked(x, y + 8, Images::Numbers_WB, number % 10);

            }
            break;

        case 100 ... 999:
            {
                uint16_t thresh = threshold;
                Sprites::drawSelfMasked(x, y + 2, Images::Numbers_WB, number / 100);
                number = number - ((number / 100) * 100);
                Sprites::drawSelfMasked(x, y + 6, Images::Numbers_WB, number / 10);
                Sprites::drawSelfMasked(x, y + 10, Images::Numbers_WB, number % 10);

            }
            break;

        case 1000 ... 9999:
            {
                Sprites::drawSelfMasked(x, y, Images::Numbers_WB, number / 1000);
                number = number - ((number / 1000) * 1000);
                Sprites::drawSelfMasked(x, y + 4, Images::Numbers_WB, number / 100);
                number = number - ((number / 100) * 100);
                Sprites::drawSelfMasked(x, y + 8, Images::Numbers_WB, number / 10);
                Sprites::drawSelfMasked(x, y + 12, Images::Numbers_WB, number % 10);

            }
            break;
    
    }

}

void drawSkull() {

    // FX::drawBitmap(76, 12, Images::Skull_Large, 0, dbmNormal);
    FX::drawBitmap(76, 12, Images::Skull_Large, skullData.getMouth(), dbmNormal);
    if (skullData.getEyes() != 255) FX::drawBitmap(99, 21, Images::Skull_Eyes, skullData.getEyes(), dbmWhite);

}

void drawLevelAndTarget(HandScore hand, uint8_t levelNumber, uint16_t target) {

    FX::drawBitmap(60, 0, Images::Background_02, 0, dbmNormal);

    drawNumber_Right(62, -2, levelNumber);
    drawNumber(62, 14, 20);
    drawNumber(62, 41, target);

}

void drawBonesMultTotal(HandScore handScore) {

    FX::drawBitmap(0, 0, Images::Background_00, 0, dbmNormal);

    if (handScore.totalBones > 0) drawNumber(44, 3, handScore.totalBones);
    if (handScore.totalMultiplier > 0) drawNumber(44, 24, handScore.totalMultiplier);
    if (handScore.score > 0) drawNumber(44, 46, handScore.score);

}


void drawFooterRoll() {

    if (hand.getPlayHandHighlight() > Constants::PlayHandHighlight_None) {

        FX::drawBitmap(0, 0, Images::Button_PlayHand, 0, dbmNormal);
        Sprites::drawSelfMasked(3, 6, Images::Numbers_BW, handsLeft);
        Sprites::drawSelfMasked(3, 16, Images::Numbers_BW, handsMax);

    }
    else {
    
        Sprites::drawSelfMasked(3, 6, Images::Numbers_WB, handsLeft);
        Sprites::drawSelfMasked(3, 16, Images::Numbers_WB, handsMax);

    }


    if (hand.getRerollHighlight() > Constants::RerollHighlight_None) {

        FX::drawBitmap(0, 39, Images::Button_Reroll, 0, dbmNormal);
        Sprites::drawSelfMasked(3, 45, Images::Numbers_BW, rerollsLeft);
        Sprites::drawSelfMasked(3, 55, Images::Numbers_BW, rerollsMax);

    }
    else {

        Sprites::drawSelfMasked(3, 45, Images::Numbers_WB, rerollsLeft);
        Sprites::drawSelfMasked(3, 55, Images::Numbers_WB, rerollsMax);

    }

    if (gameState == GameState::Game_Roll && arduboy.frameCount % 24 < 12) {

        switch (cursor) {

            case CURSOR_PLAY:

                if (hand.getPlayHandHighlight() == Constants::RerollHighlight_None) {
                    FX::drawBitmap(17, 4, Images::Skeleton_Hand, 0, dbmMasked);
                }

                break;

            case CURSOR_DECK:

                FX::drawBitmap(17, 24, Images::Skeleton_Hand, 0, dbmMasked);
                break;

            case CURSOR_REROLL:

                if (hand.getRerollHighlight() == Constants::RerollHighlight_None) {
                    FX::drawBitmap(17, 43, Images::Skeleton_Hand, 0, dbmMasked);
                }

                break;

        }

    }

}
