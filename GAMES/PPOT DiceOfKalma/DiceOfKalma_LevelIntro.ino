#include <ArduboyFX.h>  


void levelIntro() {
    
    stateTimer++;

    if (stateTimer > 60 || arduboy.justPressed(A_BUTTON)) {
        hand.rollAll();
        gameState = GameState::Game_Roll_Dice;
        returnState = GameState::Game_Roll;
    }


    FX::drawBitmap(0, 0, Images::Fire, arduboy.getFrameCount(48) / 3, dbmNormal);
    FX::drawBitmap(76, 12, Images::Skull_Large, 0, dbmNormal);
    FX::drawBitmap(0, 0 , Images::LevelEntry, 0, dbmMasked);
    drawNumber_Padded(43, 40, level, 2);
    drawNumber_Padded(32, 45, threshold, 4);
    drawNumber_Padded(21, 40, handsMax, 2);
    drawNumber_Padded(10, 43, rerollsMax, 2);

}