#include "src/utils/Arduboy2Ext.h"

static constexpr uint8_t ppotFrames = (1.6 * 30); // 1.6s animation loop

// ----------------------------------------------------------------------------
//  Handle state updates .. 
//
void splashScreen() { 

    auto justPressed = arduboy.justPressedButtons();

    if (justPressed > 0) {

        arduboy.initRandomSeed();
        gameState = GameState::Title;

    }


    uint8_t p = 69; // Default pixel position 0 is hidden in the image

    
    Sprites::drawOverwrite(51, 0, Images::PPOT, 0);

    switch (arduboy.getFrameCount(ppotFrames)) {

        case (ppotFrames*1/4) ... (ppotFrames*2/4)-1:
            p = 68;
            /*-fallthrough*/

        case 0 ... (ppotFrames * 1/4) - 1:
            Sprites::drawOverwrite(67, 58, Images::PPOT_Blank, 0);
            break;

        case (ppotFrames * 2/4) ... (ppotFrames * 3/4)-1:
            p = 67; 
            break;

        default:
            p = 66; 
            break;

    }

    arduboy.drawPixel(p, 20, WHITE);

    // On even steps of pixel movement, update the spindle image

    if (p % 2 == 0) {
        arduboy.sBuffer[68 +(1*WIDTH)] = 0x4B;
        arduboy.sBuffer[69 +(1*WIDTH)] = 0x2B;
    }

}
