#include <ArduboyFX.h>  
#include "fxdata/fxdata.h"



int8_t yOffsets[] = { 2, 2-18, 2 - 18 - 18};


void drawGame(uint8_t currentPlane) {

    for (uint8_t i = 0; i < game.getNumberOfCubes(); i++) {

        uint8_t block = game.puzzle[i] & 0xF0;
        uint8_t rotation = game.puzzle[i] & 0x0F;

        int8_t x = 0;
        int8_t y = 0;
		uint8_t grey = game.puzzle_Grey[i];

		switch (game.getPuzzleSize()) {
		
			case PuzzleSize::Small:
				x = Constants::Block_XPos_00[i / 3];
				y = Constants::Block_YPos_00[i / 3];
				break;
		
			case PuzzleSize::Large:
				x = Constants::Block_XPos_01[i / 3];
				y = Constants::Block_YPos_01[i / 3];
				break;

		}

		y = y + yOffsets[yOffset];

		if (y == -16 && i % 3 != 0) {
	        drawBlock(x, y, i % 3, block, rotation, grey, currentPlane);
		}

		if (y > -14 && y < 56) {
	        drawBlock(x, y, i % 3, block, rotation, grey, currentPlane);
		}

		if (y == 56 && i % 3 == 0) {
	        drawBlock(x, y, i % 3, block, rotation, grey, currentPlane);
		}

    }


	// Cursor ..

    if (game.getFrameCount() % 64 < 32) {

        uint8_t x = 0;
        uint8_t y = 0;

		switch (game.getPuzzleSize()) {
		
			case PuzzleSize::Small:
				x = Constants::Block_XPos_00[game.getCursor() / 3];
				y = Constants::Block_YPos_00[game.getCursor() / 3];
				break;
		
			case PuzzleSize::Large:
				x = Constants::Block_XPos_01[game.getCursor() / 3];
				y = Constants::Block_YPos_01[game.getCursor() / 3];
				break;

		}

		y = y + yOffsets[yOffset];

		if (y > -16 && y < 42) {

			switch (game.getCursor() % 3) {

				case 0:
					SpritesU::drawPlusMaskFX(x, y, Images::Cursor_Top, currentPlane);
					break;

				case 1:
					SpritesU::drawPlusMaskFX(x, y + 6, Images::Cursor_Left, currentPlane);
					break;

				case 2:
					SpritesU::drawPlusMaskFX(x + 11, y + 6, Images::Cursor_Right, currentPlane);
					break;

			}

		}

    }

}


void drawBlock(int8_t x, int8_t y, uint8_t face, uint8_t block, uint8_t rotation, uint8_t grey, uint8_t currentPlane) {
    
	switch (face) {

		case Constants::Cube_Top:

			switch (block) {
				
				case Constants::Block_Knob:
					SpritesU::drawPlusMaskFX(x, y, Images::Block_Top_01, (3 * rotation) + (grey * 3 * 4) + currentPlane);
					break;

				case Constants::Block_Line:
					SpritesU::drawPlusMaskFX(x, y, Images::Block_Top_02, (3 * rotation) + (grey * 3 * 4) + currentPlane);
					break;

				case Constants::Block_Tee:
					SpritesU::drawPlusMaskFX(x, y, Images::Block_Top_03, (3 * rotation) + (grey * 3 * 4) + currentPlane);
					break;

				case Constants::Block_Curve:
					SpritesU::drawPlusMaskFX(x, y, Images::Block_Top_04, (3 * rotation) + (grey * 3 * 4) + currentPlane);
					break;

			}

			break;

		case Constants::Cube_Left:
			
			switch (block) {
			
				case Constants::Block_Knob:
				SpritesU::drawPlusMask(x, y + 6, Images::Block_Left_01, (3 * rotation) + (grey * 3 * 4) + currentPlane);
				break;

				case Constants::Block_Line:
				SpritesU::drawPlusMask(x, y + 6, Images::Block_Left_02, (3 * rotation) + (grey * 3 * 4) + currentPlane);
				break;

				case Constants::Block_Tee:
				SpritesU::drawPlusMask(x, y + 6, Images::Block_Left_03, (3 * rotation) + (grey * 3 * 4) + currentPlane);
				break;

				case Constants::Block_Curve:
				SpritesU::drawPlusMask(x, y + 6, Images::Block_Left_04, (3 * rotation) + (grey * 3 * 4) + currentPlane);
				break;

			}

			break;

		case Constants::Cube_Right:
			
			switch (block) {
			
				case Constants::Block_Knob:
				SpritesU::drawPlusMask(x + 11, y + 6, Images::Block_Right_01, (3 * rotation) + (grey * 3 * 4) + currentPlane);
				break;

				case Constants::Block_Line:
				SpritesU::drawPlusMask(x + 11, y + 6, Images::Block_Right_02, (3 * rotation) + (grey * 3 * 4) + currentPlane);
				break;

				case Constants::Block_Tee:
				SpritesU::drawPlusMask(x + 11, y + 6, Images::Block_Right_03, (3 * rotation) + (grey * 3 * 4) + currentPlane);
				break;

				case Constants::Block_Curve:
				SpritesU::drawPlusMask(x + 11, y + 6, Images::Block_Right_04, (3 * rotation) + (grey * 3 * 4) + currentPlane);
				break;

			}

			break;

	}

}