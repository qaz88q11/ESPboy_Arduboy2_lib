#include <ArduboyFX.h>  
#include "src/utils/Constants.h" 

// void setUpgradeTop() {

//     switch (upgradeCursor) {

//         case 0 ... 1:
//             upgradeTop = 0;
//             break;

//         case 2:
//             upgradeTop = 1;
//             break;

//         case 3:
//             upgradeTop = 2;
//             break;

//         case 4:
//             upgradeTop = 3;
//             break;

//         case 5:
//             upgradeTop = 4;
//             break;

//         default:
//             upgradeTop = 5;
//             break;

//     }


// }

void upgradeHand_Init() {
  
    hand.selectUpgradeHands(3);
    gameState = GameState::Game_Upgrade_Choice;

}

void upgradeHand() {

    uint8_t x = 84;
    uint8_t ugIdx = 0;
    UpgradeHand_BottomOptions botImage = UpgradeHand_BottomOptions::Both;


    // Handle input ..

    if (arduboy.justPressed(RIGHT_BUTTON) && upgradeCursor > 0) {
        upgradeCursor = upgradeCursor - 1;
        // setUpgradeTop();
    }
    else if (arduboy.justPressed(LEFT_BUTTON) && upgradeCursor < 2) {
        upgradeCursor = upgradeCursor + 1;
        // setUpgradeTop();
    }
    else if (arduboy.justPressed(A_BUTTON)) {
        hand.setUpgradeHand(hand.getUpgradeHandType(upgradeCursor));
        startLevel(true);
    }


    // Render screen ..

    if (upgradeCursor > 0) { 

        if (upgradeCursor == 2) {
            botImage = UpgradeHand_BottomOptions::UpOnly;
        }
        else {
            botImage = UpgradeHand_BottomOptions::Both;
        }

    }
    else { 

        if (upgradeCursor == 2) {
            botImage = UpgradeHand_BottomOptions::Both;
        }
        else {
            botImage = UpgradeHand_BottomOptions::DownOnly;
        }

    }
    
    for (uint8_t i = 0; i < 3; i++) {

        uint8_t handToUpgrade = hand.getUpgradeHand(2 - i);

        if (i == upgradeCursor) {
            FX::drawBitmap(x, 0, Images::UpgradeHands, handToUpgrade + 8 + (arduboy.frameCount % 24 < 12 ? 0 : 8), dbmNormal);
        }
        else {
            FX::drawBitmap(x, 0, Images::UpgradeHands, handToUpgrade, dbmNormal);
        }

        x = x - 33;

    }

    FX::drawBitmap(0, 0, Images::UpgradeHand_Bot, static_cast<uint8_t>(botImage), dbmNormal);
    FX::drawBitmap(115, 0, Images::UpgradeHand_Top, 0, dbmNormal);

    if (arduboy.frameCount % 24 < 12) {

        FX::drawBitmap(84 - 4 - ((upgradeCursor - upgradeTop) * 33), 0, Images::UpgradeHand_Cursor, 0, dbmWhite);
        
    }
    
}