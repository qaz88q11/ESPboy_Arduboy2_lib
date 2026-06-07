#include <ArduboyFX.h>  
#include "src/utils/ArduboyG.h"
#include "src/utils/Constants.h"
#include "src/utils/Enums.h"
#include "fxdata/fxdata.h"
#include "src/utils/SpritesU.hpp"


void title_Init() {

    gameState = GameState::Title_Main;
    titleCounter = 0;
    game.resetFrameCount();

}

void title_Update() {

    game.incFrameCount();

    uint8_t justPressed = getJustPressedButtons();
    uint8_t aReleased = getJustReleasedButtons(A_BUTTON);
    uint8_t bReleased = getJustReleasedButtons(B_BUTTON);
    uint8_t pressed = getPressedButtons();

    switch (gameState) {

        case GameState::Title_Main:

            if ((game.getFrameCount() > 24) && (justPressed & A_BUTTON)) {

                a.initRandomSeed(); 
                gameState = GameState::Title_Size;
                levelSelect.setACounter(11);
                titleCounter = 0;

            }
            break;

        case GameState::Title_Size:

            if (justPressed & UP_BUTTON && game.getPuzzleSize() == PuzzleSize::Large) {
                game.setPuzzleSize(PuzzleSize::Small);
                saveCookie();
            }

            if (justPressed & DOWN_BUTTON && game.getPuzzleSize() == PuzzleSize::Small) {
                game.setPuzzleSize(PuzzleSize::Large);
                saveCookie();
            }

            if ((game.getFrameCount() > 24) && (justPressed & A_BUTTON)) {

                uint8_t nextGame = 0;

                for (uint8_t i = 0; i < Constants::Level_Count; i++) {

                    nextGame = i;
                    break;

                }

                levelSelect.setGame(nextGame);
                a.initRandomSeed(); 
                gameState = GameState::Title_Select;
                levelSelect.setACounter(11);
                titleCounter = 0;

            }
            break;

        case GameState::Title_Select:

            if (bReleased && levelSelect.getBCounter() < 10) {
                gameState = GameState::Title_Init;
            }

            if (aReleased && levelSelect.getACounter() < 10) {
                game.setLevel(levelSelect.getSelectedPuzzle());
                gameState = GameState::Play_Init;
            }

            if (pressed & A_BUTTON) {
                levelSelect.incACounter();
            }
            else {
                levelSelect.setACounter(0);
            }

            if (pressed & B_BUTTON) {
                levelSelect.incBCounter();
            }
            else {
                levelSelect.setBCounter(0);
            }

            if (levelSelect.getACounter() > 32 && levelSelect.getBCounter() > 32) {
                gameState = GameState::Title_Clear_Progress;      
            }

            if (justPressed & LEFT_BUTTON && levelSelect.getX() > 0) {
                levelSelect.setX(levelSelect.getX() - 1);
            }

            if (justPressed & RIGHT_BUTTON && levelSelect.getX() < 3 && game.getPuzzle(static_cast<uint8_t>(game.getPuzzleSize()), levelSelect.getSelectedPuzzle() + 1).getStatus() != PuzzleStatus::Locked) {
                levelSelect.setX(levelSelect.getX() + 1);
            }

            if (justPressed & DOWN_BUTTON && levelSelect.getY() < 5 && game.getPuzzle(static_cast<uint8_t>(game.getPuzzleSize()), levelSelect.getSelectedPuzzle() + 4).getStatus() != PuzzleStatus::Locked) {
                levelSelect.setY(levelSelect.getY() + 1);
            }

            if (justPressed & UP_BUTTON && levelSelect.getY() > 0) {
                levelSelect.setY(levelSelect.getY() - 1);
            }

            break;

        case GameState::Title_Clear_Progress:

            if (justPressed & A_BUTTON) {

                cookieReset(static_cast<uint8_t>(game.getPuzzleSize()));
                saveCookie();
                levelSelect.setACounter(11);
                levelSelect.setBCounter(11);   
                gameState = GameState::Title_Select;

            }

            if (justPressed & B_BUTTON) {

                gameState = GameState::Title_Select;

            }

            break;

    }

    if (titleCounter < 254) {
        titleCounter++;
    }

}

void title(ArduboyGBase_Config<ABG_Mode::L4_Triplane> &a) {
       
    if (a.needsUpdate()) title_Update();

    uint8_t currentPlane = a.currentPlane();
    uint8_t yOffset = Constants::levelSelect_Offset[levelSelect.getY()];
    uint8_t imageIdx;

    switch (gameState) {

        case GameState::Title_Main:

            imageIdx = 13 - (game.getFrameCount() % 140) / 10;
            SpritesU::drawOverwriteFX(35, 0, Images::Rotate, (imageIdx * 3) + currentPlane);
            SpritesU::drawPlusMaskFX(0, 25, Images::Title, currentPlane);
            break;

        case GameState::Title_Size:

            imageIdx = 13 - (game.getFrameCount() % 140) / 10;
            SpritesU::drawOverwriteFX(35, 0, Images::Rotate, (imageIdx * 3) + currentPlane);
            SpritesU::drawPlusMaskFX(20, 12, Images::Size, (static_cast<uint8_t>(game.getPuzzleSize()) * 3) + currentPlane);
            break;

        case GameState::Title_Select:
            
            for (uint8_t y = 0; y < 6; y++) {

                if (((y * 18) - yOffset < -18) || ((y * 18) - yOffset) >= 63) continue;

                for (uint8_t x = 0; x < 4; x++) {

                    Puzzle &puzzle = game.getPuzzle(static_cast<uint8_t>(game.getPuzzleSize()), (y * 4) + x);

                    if (puzzle.getStatus() == PuzzleStatus::Complete) {

                        SpritesU::drawOverwriteFX(x * 19, (y * 18) - yOffset, Images::Levels_Select, (((y * 4) + x) * 3) + currentPlane);

                    }
                    else if (puzzle.getStatus() == PuzzleStatus::InProgress) {

                        SpritesU::drawOverwriteFX(x * 19, (y * 18) - yOffset, Images::Levels_Select, ((((y * 4) + x) + 28) * 3) + currentPlane);

                    }                        
                    else {

                        SpritesU::drawOverwriteFX(x * 19, (y * 18) - yOffset, Images::Levels_Select, (52 * 3) + currentPlane);

                    }

                    if (((y * 4) + x) == levelSelect.getSelectedPuzzle()) {

                        if (game.getFrameCount(48)) {
                            SpritesU::drawPlusMaskFX(x * 19, (y * 18) - yOffset, Images::Levels_Cursor, currentPlane);
                        }

                    }

                }

                SpritesU::drawOverwriteFX(128 - 53, 0, Images::Levels_HUD, (levelSelect.getY() * 3) + currentPlane);
                SpritesU::drawOverwriteFX(128 - 53 + 40, 2, Images::Levels_Number, (levelSelect.getSelectedPuzzle() * 3) + currentPlane);
                SpritesU::drawOverwriteFX(128 - 53 + 9, 17, Images::Levels_Status, (static_cast<uint8_t>(game.getPuzzle(static_cast<uint8_t>(game.getPuzzleSize()), levelSelect.getSelectedPuzzle()).getStatus()) * 3) + currentPlane);

                if (game.getPuzzle(static_cast<uint8_t>(game.getPuzzleSize()), levelSelect.getSelectedPuzzle()).getStatus() == PuzzleStatus::Complete) {

                    uint16_t time = game.getPuzzle(static_cast<uint8_t>(game.getPuzzleSize()), levelSelect.getSelectedPuzzle()).getTime();
                    SpritesU::drawOverwriteFX(128 - 53 + 7, 32, Images::Levels_Time, currentPlane);
                    SpritesU::drawOverwriteFX(106, 32, Images::Levels_Time_Numbers, ((time / 1000) * 3) + currentPlane);
                    SpritesU::drawOverwriteFX(111, 32, Images::Levels_Time_Numbers, (((time % 1000) / 100) * 3) + currentPlane);
                    SpritesU::drawOverwriteFX(116, 32, Images::Levels_Time_Numbers, (((time % 100) / 10) * 3) + currentPlane);
                    SpritesU::drawOverwriteFX(121, 32, Images::Levels_Time_Numbers, (((time % 100) % 10) * 3) + currentPlane);

                }
                
            }

            break;

        case GameState::Title_Clear_Progress:
            SpritesU::drawOverwriteFX(36, 25, Images::ClearProgress, currentPlane);
            break;

    }            

}
