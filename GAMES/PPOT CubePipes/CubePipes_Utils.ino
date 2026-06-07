#include <ArduboyFX.h>  
#include "src/utils/ArduboyG.h"
#include "src/utils/Constants.h"
#include "src/utils/Enums.h"
#include "fxdata/fxdata.h"
#include "src/utils/SpritesU.hpp"


uint8_t getJustPressedButtons() {

    a.pollButtons();

    return a.justPressedButtons();

}

uint8_t getPressedButtons() {

    return a.pressedButtons();

}

uint8_t getJustReleasedButtons(uint8_t button) {

    return a.justReleasedButtons(button);

}

void saveCookie() {

    FX::saveGameState(cookie);

}

void cookieReset(uint8_t size) {

    for (uint8_t i = 0; i < 24; i++) {

        #ifdef _DEBUG
            if (i < 23) { //SJH
                game.getPuzzle(size, i).setStatus(PuzzleStatus::InProgress);
                game.getPuzzle(size, i).setTime(0);
            }
            else {
                game.getPuzzle(size, i).setStatus(PuzzleStatus::Locked);
                game.getPuzzle(size, i).setTime(0);
            }
        #endif

        #ifdef DEBUG
            if (i < 23) { 
                game.getPuzzle(size, i).setStatus(PuzzleStatus::Complete);
                game.getPuzzle(size, i).setTime(i);
            }
            else {
                game.getPuzzle(size, i).setStatus(PuzzleStatus::InProgress);
                game.getPuzzle(size, i).setTime(0);
            }
        #endif

        #ifndef DEBUG
            if (i == 0) { 
                game.getPuzzle(size, i).setStatus(PuzzleStatus::InProgress);
                game.getPuzzle(size, i).setTime(0);
            }
            else {
                game.getPuzzle(size, i).setStatus(PuzzleStatus::Locked);
                game.getPuzzle(size, i).setTime(0);
            }
        #endif

    }
   

}