#include <ArduboyFX.h>  

void gameOver() {

    if (arduboy.justPressed(A_BUTTON)) {
        gameState = GameState::Title;
    }

    FX::drawBitmap(0, 0, Images::Fire, arduboy.getFrameCount(48) / 3, dbmNormal);
    FX::drawBitmap(76, 12, Images::Skull_Large, skullData.getMouth(), dbmNormal);
    FX::drawBitmap(0, 0, Images::GameOver, 0, dbmMasked);
    arduboy.fillRect(27, 41 + (level < 10 ? 4 : 0), 7, 5 + (level >= 10 ? 4 : 0), BLACK);
    drawNumber_Right(28, 34, level);
    arduboy.fillRect(20, 41 + (cookie.getBestLevel() < 10 ? 4 : 0), 7, 5 + (cookie.getBestLevel() >= 10 ? 4 : 0), BLACK);
    drawNumber_Right(21, 34, cookie.getBestLevel());
    arduboy.fillRect(13, 41 + (hand.getDeckCount() < 10 ? 4 : 0), 7, 5 + (hand.getDeckCount() >= 10 ? 4 : 0), BLACK);
    drawNumber_Right(14, 34, hand.getDeckCount());

}

void win() {

    if (arduboy.justPressed(A_BUTTON)) {
        gameState = GameState::Title;
    }

    FX::drawBitmap(0, 0, Images::Fire, arduboy.getFrameCount(48) / 3, dbmNormal);
    FX::drawBitmap(0, 0, Images::GameWin, 0, dbmMasked);
    drawNumber(36, 13, hand.getDeckCount());

}