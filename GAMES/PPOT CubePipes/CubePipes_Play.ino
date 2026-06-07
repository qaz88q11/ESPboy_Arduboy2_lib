#include <ArduboyFX.h>  
#include "fxdata/fxdata.h"
#include "src/entities/Puzzles.h"

GameOver endOfGame = GameOver::No;
uint16_t bPressed = 0;

void checkResult() {

    uint8_t completed = 0;

	for (uint8_t i = 0; i < game.getNumberOfCubes(); i++) {
	
        uint8_t v = 0;

        switch (game.getPuzzleSize()) {
        
            case PuzzleSize::Small:
                v = pgm_read_byte(&puzzles_00[game.getLevel()][i]);
                break;
        
            case PuzzleSize::Large:
                v = pgm_read_byte(&puzzles_01[game.getLevel()][i]);
                break;

        }

		if (game.puzzle[i] != v) {
		
			endOfGame = GameOver::No;
			return;

		}

	}


    // Complete ?

    game.getPuzzle(static_cast<uint8_t>(game.getPuzzleSize()), game.getLevel()).setStatus(PuzzleStatus::Complete);
    game.getPuzzle(static_cast<uint8_t>(game.getPuzzleSize()), game.getLevel()).setTime(game.getTime());

    for (uint8_t i = 0; i < Constants::Level_Count; i++) {

        if (game.getPuzzle(static_cast<uint8_t>(game.getPuzzleSize()), i).getStatus() == PuzzleStatus::Complete) {
            completed++;
        }

    }

    if (completed == 24) {

    	endOfGame = GameOver::GameOver;
        return;

    }

    endOfGame = GameOver::LevelOver;

}

void play_Init() { 

    popoutMenu.setSelect(0);
    popoutMenu.setX(128);
    popoutMenu.setDirection(Direction::None);

    gameState = GameState::Play;
    game.setTime(0);
    endOfGame = GameOver::No;


    switch (game.getPuzzleSize()) {
    
        case PuzzleSize::Small:
            memcpy_P(game.puzzle, puzzles_00[game.getLevel()], game.getNumberOfCubes());
            break;
    
        case PuzzleSize::Large:
            memcpy_P(game.puzzle, puzzles_01[game.getLevel()], game.getNumberOfCubes());
            break;
    
    }

    game.setCursor(0);

    
    // Mess up puzzle ..

	for (uint8_t i = 0; i < game.getNumberOfCubes(); i++) { 
	
        if ((game.puzzle[i] & 0xF0) == Constants::Block_Line) {

    		game.puzzle[i] = (game.puzzle[i] & 0xf0) | (0x0f & static_cast<uint8_t>(random(0,2)));

        }
        else {

    		game.puzzle[i] = (game.puzzle[i] & 0xf0) | (0x0f & static_cast<uint8_t>(random(0,4)));

        }
        
	}


    // Copy current puzzle into restore array ..

	for (uint8_t i = 0; i < game.getNumberOfCubes(); i++) {
	
		game.puzzle_Orig[i] = game.puzzle[i];
		game.puzzle_Grey[i] = 0;
        
	}

}

void handleMenu(uint8_t justPressed) {

    if (justPressed & UP_BUTTON) {

        if (popoutMenu.getSelect() >= (game.getUndoCount() == 0 ? 2 : 1)) {
            popoutMenu.setSelect(popoutMenu.getSelect() - 1);
        }

    }

    else if (justPressed & DOWN_BUTTON) {

        if (popoutMenu.getSelect() < 2) {
            popoutMenu.setSelect(popoutMenu.getSelect() + 1);
        }

    }

    else if (justPressed & A_BUTTON) {

        switch (popoutMenu.getSelect()) {
        
            case 0:
                game.revertMove();
                popoutMenu.setAllowClose(true);
                if (game.getUndoCount() == 0) {
                    popoutMenu.setDirection(Direction::Right);
                }
                break;
        
            case 1:
                popoutMenu.setAllowClose(true);
                game.setCursor(0);
                memcpy(game.puzzle, game.puzzle_Orig, game.getNumberOfCubes());
                popoutMenu.setDirection(Direction::Right);
                
                break;
        
            case 2:
                popoutMenu.setAllowClose(true);
                gameState = GameState::Title_Init;
                break;
                
        }

    }

    else if (justPressed & B_BUTTON) {

        popoutMenu.setAllowClose(true);
        popoutMenu.setDirection(Direction::Right);

    }

}


void play_Update() { 

    uint8_t justPressed = getJustPressedButtons();
    uint8_t pressed = getPressedButtons();

    game.incFrameCount();


    if (gameState == GameState::Play) {

        if (popoutMenu.getX() == 128) {

            if (endOfGame == GameOver::No) {

                if (game.getFrameCount() % 52 == 0) {
                    game.incTime();
                }

                if (justPressed & A_BUTTON) {

                    game.captureMove();

                    switch (game.puzzle[game.getCursor()] & 0xF0) {
                    
                        case Constants::Block_Line:
                            game.puzzle[game.getCursor()] = (game.puzzle[game.getCursor()] & 0xF0) + (((game.puzzle[game.getCursor()] & 0x0F) + 1 ) % 2);
                            break;

                        default:
                            game.puzzle[game.getCursor()] = (game.puzzle[game.getCursor()] & 0xF0) + (((game.puzzle[game.getCursor()] & 0x0F) + 1 ) % 4);
                            break;
                    
                    }

                    checkResult();

                    if (endOfGame != GameOver::No) {
                        justPressed = 0;
                    }

                }

                if (pressed & B_BUTTON) {
                
                    bPressed++;

                }

                if (a.notPressed(B_BUTTON)) {

                    if (bPressed > 0 && bPressed < 10) {

                        game.puzzle_Grey[game.getCursor()] = !game.puzzle_Grey[game.getCursor()];

                    }

                }
                        

                uint8_t cursorMod3 = game.getCursor() % 3;

                uint8_t moves_Top = 0;
                uint8_t moves_Left = 0;
                uint8_t moves_Right = 0;

                switch (game.getPuzzleSize()) {
                
                    case PuzzleSize::Small:
                        moves_Top = Constants::Valid_Moves_00[game.getCursor() / 3][Constants::Cube_Top];
                        moves_Left = Constants::Valid_Moves_00[game.getCursor() / 3][Constants::Cube_Left];
                        moves_Right = Constants::Valid_Moves_00[game.getCursor() / 3][Constants::Cube_Right];
                        break;
                
                    case PuzzleSize::Large:
                        moves_Top = Constants::Valid_Moves_01[game.getCursor() / 3][Constants::Cube_Top];
                        moves_Left = Constants::Valid_Moves_01[game.getCursor() / 3][Constants::Cube_Left];
                        moves_Right = Constants::Valid_Moves_01[game.getCursor() / 3][Constants::Cube_Right];
                        break;
                    
                }


                if (a.justPressed(UP_BUTTON)) {
                
                    if (cursorMod3 == Constants::Cube_Top && (moves_Top & Constants::Direction_Top_UL || moves_Top & Constants::Direction_Top_UR)) {
                    
                        if (a.pressed(LEFT_BUTTON) && moves_Top & Constants::Direction_Top_UL) {

                            game.incCursor(-7);
                            game.setFrameCount(0);

                        }
                        else if (a.pressed(RIGHT_BUTTON) && moves_Top & Constants::Direction_Top_UR) {
                            switch (game.getPuzzleSize()) {

                                case PuzzleSize::Small:

                                    switch (game.getCursor()) {
                                    
                                        case 6:
                                        case 9:
                                        case 12:
                                            game.incCursor(-5);
                                            game.setFrameCount(0);
                                            break;

                                        case 15:
                                        case 18:
                                            game.incCursor(-8);
                                            game.setFrameCount(0);
                                            break;

                                    }

                                    break;

                                case PuzzleSize::Large:

                                    switch (game.getCursor()) {
                                    
                                        case 9:
                                        case 12:
                                        case 15:

                                            game.incCursor(-8);
                                            game.setFrameCount(0);
                                            break;

                                        case 21:
                                        case 24:
                                        case 27:
                                        case 30:

                                            game.incCursor(-11);
                                            game.setFrameCount(0);
                                            break;

                                    }

                                    break;

                            }

                        }
                        else {
                            switch (game.getPuzzleSize()) {

                                case PuzzleSize::Small:

                                    switch (game.getCursor()) {
                                    
                                        case 6:
                                            game.incCursor(-5);
                                            game.setFrameCount(0);
                                            break;

                                        case 9:
                                        case 12:
                                            game.incCursor(-7);
                                            game.setFrameCount(0);
                                            break;

                                        case 15:
                                        case 18:
                                            game.incCursor(-7);
                                            game.setFrameCount(0);
                                            break;

                                    }

                                    break;

                                case PuzzleSize::Large:

                                    switch (game.getCursor()) {
                                    
                                        case 6:
                                            game.incCursor(-5);
                                            game.setFrameCount(0);
                                            break;

                                        case 9:
                                            game.incCursor(-8);
                                            game.setFrameCount(0);
                                            break;

                                        case 12:
                                        case 15:
                                        case 18:
                                            game.incCursor(-10);
                                            game.setFrameCount(0);
                                            break;

                                        case 21:
                                            game.incCursor(-11);
                                            game.setFrameCount(0);
                                            break;

                                        case 24:
                                        case 27:
                                        case 30:
                                        case 33:
                                            game.incCursor(-13);
                                            game.setFrameCount(0);
                                            break;

                                        case 36:
                                        case 39:
                                        case 42:
                                        case 45:
                                            game.incCursor(-13);
                                            game.setFrameCount(0);
                                            break;

                                        case 48:
                                        case 51:
                                        case 54:
                                            game.incCursor(-10);
                                            game.setFrameCount(0);
                                            break;

                                    }

                                    break;

                            }


                        }

                    }

                    else if (cursorMod3 == Constants::Cube_Left && moves_Left & Constants::Direction_Side_U) {

                        game.incCursor(-1);
                        game.setFrameCount(0);

                    }
                    
                    else if (cursorMod3 == Constants::Cube_Right && moves_Right & Constants::Direction_Side_U) {

                        game.incCursor(-2);
                        game.setFrameCount(0);

                    }

                }

                else if (a.justPressed(DOWN_BUTTON)) {
                
                    if (cursorMod3 == Constants::Cube_Top && (moves_Top & Constants::Direction_Top_DL || moves_Top & Constants::Direction_Top_DR)) {
                    
                        if (a.pressed(LEFT_BUTTON) && moves_Top & Constants::Direction_Top_DL) {

                            game.incCursor(1);
                            game.setFrameCount(0);

                        }
                        else if (a.pressed(RIGHT_BUTTON) && moves_Top & Constants::Direction_Top_DR) {

                            game.incCursor(2);
                            game.setFrameCount(0);

                        }
                        else {

                            game.incCursor(1);
                            game.setFrameCount(0);

                        }

                    }

                    else if (cursorMod3 == Constants::Cube_Left && moves_Left & Constants::Direction_Side_D) {

                        switch (game.getPuzzleSize()) {

                            case PuzzleSize::Small:
                                game.incCursor(5);
                                game.setFrameCount(0);
                                break;

                            case PuzzleSize::Large:

                                switch (game.getCursor()) {

                                    case 1:
                                    case 4:
                                    case 7:
                                        game.incCursor(8);
                                        game.setFrameCount(0);
                                        break;

                                    case 10:
                                    case 13:
                                    case 16:
                                    case 19:
                                        game.incCursor(11);
                                        game.setFrameCount(0);
                                        break;

                                    case 25:
                                    case 28:
                                    case 31:
                                    case 34:
                                        game.incCursor(11);
                                        game.setFrameCount(0);
                                        break;

                                    case 40:
                                    case 43:
                                    case 46:
                                        game.incCursor(8);
                                        game.setFrameCount(0);
                                        break;
                                }

                                break;

                        }

                    }
                    
                    else if (cursorMod3 == Constants::Cube_Right && moves_Right & Constants::Direction_Side_D) {

                        switch (game.getPuzzleSize()) {

                            case PuzzleSize::Small:
                                game.incCursor(7);
                                game.setFrameCount(0);
                                break;

                            case PuzzleSize::Large:

                                switch (game.getCursor()) {

                                    case 2:
                                    case 5:
                                    case 8:
                                        game.incCursor(10);
                                        game.setFrameCount(0);
                                        break;

                                    case 11:
                                    case 14:
                                    case 17:
                                    case 20:
                                        game.incCursor(13);
                                        game.setFrameCount(0);
                                        break;

                                    case 23:
                                        game.incCursor(13);
                                        game.setFrameCount(0);
                                        break;

                                    case 26:
                                    case 29:
                                    case 32:
                                        game.incCursor(13);
                                        game.setFrameCount(0);
                                        break;

                                    case 38:
                                    case 41:
                                    case 44:
                                        game.incCursor(10);
                                        game.setFrameCount(0);
                                        break;

                                }                                
                                break;

                        }

                    }

                }

                else if (a.justPressed(RIGHT_BUTTON)) {

                    if (cursorMod3 == Constants::Cube_Top && moves_Top & Constants::Direction_Top_R) {

                        game.incCursor(3);
                        game.setFrameCount(0);

                    }
                    else if (cursorMod3 == Constants::Cube_Left) {

                        game.incCursor(1);
                        game.setFrameCount(0);

                    }
                    else if (cursorMod3 == Constants::Cube_Right && moves_Right & Constants::Direction_Side_R) {

                        game.incCursor(2);
                        game.setFrameCount(0);

                    }

                }


                else if (a.justPressed(LEFT_BUTTON)) {

                    if (cursorMod3 == Constants::Cube_Top && moves_Top & Constants::Direction_Top_L) {

                        game.incCursor(-3);
                        game.setFrameCount(0);

                    }
                    else if (cursorMod3 == Constants::Cube_Right) {

                        game.incCursor(-1);
                        game.setFrameCount(0);

                    }
                    else if (cursorMod3 == Constants::Cube_Left && moves_Left & Constants::Direction_Side_L) {

                        game.incCursor(-2);
                        game.setFrameCount(0);

                    }

                }
            }

            // Have we won?

            else {

                if (justPressed & A_BUTTON) {

                    endOfGame = GameOver::No;
                
                    game.getPuzzle(static_cast<uint8_t>(game.getPuzzleSize()), game.getLevel()).setStatus(PuzzleStatus::Complete);
                    game.getPuzzle(static_cast<uint8_t>(game.getPuzzleSize()), game.getLevel()).setTime(game.getTime());
                    
                    uint8_t completed = 0;
                    uint8_t nextGame = 255;

                    for (uint8_t i = 0; i < Constants::Level_Count; i++) {

                        if (game.getPuzzle(static_cast<uint8_t>(game.getPuzzleSize()), i).getStatus() == PuzzleStatus::Complete) {
                            completed++;
                        }
                        else {
                            if (nextGame == 255) nextGame = i;
                        }

                    }

                    if (completed == 24) {

                        gameState = GameState::Title_Init;

                    }
                    else {

                        gameState = GameState::Play_Init;
                        game.getPuzzle(static_cast<uint8_t>(game.getPuzzleSize()), nextGame).setStatus(PuzzleStatus::InProgress);
                        game.setLevel(nextGame);
                        levelSelect.setGame(nextGame);

                    }

                    saveCookie();

                }

            }

        }
        else {

            handleMenu(justPressed);

        }


        // Calculate y offset ..

        if (game.getPuzzleSize() == PuzzleSize::Small) {
        
            yOffset = 0;
            
        }
        else {
        
            switch (game.getCursor()) {
            
                case 0 ... 20:
                    yOffset = 0;
                    break;

                case 21 ... 35:
                    yOffset = 1;
                    break;

                default:
                    yOffset = 2;
                    break;
            
            }

        }


            
        // Open menu ..

        if (a.notPressed(B_BUTTON) & bPressed > 10) {
                    
            if (popoutMenu.getX() == 128) {
                popoutMenu.setDirection(Direction::Left);
                popoutMenu.setSelect(game.getUndoCount() == 0 ? 1 : 0); 
            }
            else if (popoutMenu.getX() == 128 - 32 && popoutMenu.getAllowClose()) {
                popoutMenu.setDirection(Direction::Right);
            }
            
        }

        switch (popoutMenu.getDirection()) {
        
            case Direction::Left:

                popoutMenu.setX(popoutMenu.getX() - 2);

                if (popoutMenu.getX() == 128 - 32) {
                    popoutMenu.setDirection(Direction::None);
                }

                break;
        
            case Direction::Right:

                popoutMenu.setX(popoutMenu.getX() + 2);

                if (popoutMenu.getX() == 128) {
                    popoutMenu.setDirection(Direction::None);
                }

                break;
            
        }

    }
    else {

        gameState = GameState::Play_Init;
        game.getPuzzle(static_cast<uint8_t>(game.getPuzzleSize()), game.getLevel() + 1).setStatus(PuzzleStatus::InProgress);
        game.setLevel(game.getLevel() + 1);
        levelSelect.increaseGame();
    
    }


    if (a.notPressed(B_BUTTON)) {

        bPressed = 0;

    }

}


void play(ArduboyGBase_Config<ABG_Mode::L4_Triplane> &a) {

    uint8_t currentPlane = a.currentPlane();
    if (a.needsUpdate()) { play_Update(); }


    // Draw Mini Hud ..

    drawGame(a.currentPlane());

    SpritesU::drawOverwriteFX(120, 0, Images::Mini_HUD, currentPlane);
    SpritesU::drawOverwriteFX(121, 17, Images::Numbers_HUD, ((game.getLevel() + 1) * 3) + currentPlane);
    SpritesU::drawOverwriteFX(121, 46, Images::Numbers_HUD, ((game.getTime() / 100) * 3) + currentPlane);
    SpritesU::drawOverwriteFX(121, 54, Images::Numbers_HUD, ((game.getTime() % 100) * 3) + currentPlane);


    if (popoutMenu.getX() < 128) {
        SpritesU::drawOverwriteFX(popoutMenu.getX(), 0, Images::Menu, ((popoutMenu.getSelect() + (game.getUndoCount() == 0 ? 2 : 0)) * 3) + ((popoutMenu.getAllowClose() ? 0 : 5) * 3) + currentPlane);
    }


	if (endOfGame == GameOver::LevelOver) {
		SpritesU::drawPlusMaskFX(5, 22, Images::Complete, currentPlane);
	}

	if (endOfGame == GameOver::GameOver) {
		SpritesU::drawPlusMaskFX(0, 22, Images::GameOver, currentPlane);
	}

}
