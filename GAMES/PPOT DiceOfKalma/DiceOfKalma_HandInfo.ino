#include <ArduboyFX.h>  
#include "src/utils/Constants.h" 

int16_t handScore_xMax = 0;
int16_t handScore_x = 0;

void handInfo_Init() {

    handScore_x = Constants::HandScore_StartingPos;
    gameState = GameState::Game_Hand_Info;
    addLongTalk();
    arduboy.frameCount = 1;
    messageIdx = random(0, 4);

}

void handInfo() {

    if (arduboy.getFrameCount(96) == 0) addPinPrick();


    UpgradeHand_BottomOptions botImage = UpgradeHand_BottomOptions::Both;


    // Handle input ..

    if (arduboy.justPressed(A_BUTTON)) {
        gameState = GameState::Game_Roll;
    }

    if (arduboy.pressed(LEFT_BUTTON)) {
        if (handScore_x < handScore_xMax) {
            handScore_x = handScore_x + 2;
        }
    
    }

    if (arduboy.pressed(RIGHT_BUTTON)) {
        if (handScore_x > Constants::HandScore_StartingPos) {
            handScore_x = handScore_x - 2;
        }
    
    }

    int16_t x = handScore_x;

    
    // Render screen ..
    

    FX::drawBitmap(x, 4, Images::Bones_Heading, 0, dbmNormal);
    drawNumber_Right(x, 45, hand.getLastHandScore().baseBones);
    x = x - Constants::HandScore_LineSpacing;


    // Hand Bones ..

    if (hand.getLastHandScore().handType != HandType::None) {

        FX::drawBitmap(x, 4, Images::Hand_Names, static_cast<uint8_t>(hand.getLastHandScore().handType), dbmNormal);
        drawNumber_Right(x, 45, hand.getLastHandScore().handBones);
        x = x - Constants::HandScore_LineSpacing;
    
    }


    // Skull Bones ..

    for (uint8_t i = 0; i < hand.getDeckCount(); i++) {

        if (hand.getDeckEntry(i).getBones() > 0) {
            FX::drawBitmap(x - 1, 4, Images::Skull_Names, static_cast<uint8_t>(hand.getDeckEntry(i).getSkullType()), dbmNormal);
            drawNumber_Right(x, 45, hand.getDeckEntry(i).getBones());
            x = x - Constants::HandScore_LineSpacing;
        }

    }


    // Upgrade Bones ..

    if (hand.getLastHandScore().upgradeBones > 0) {
        FX::drawBitmap(x, 4, Images::Upgrade, 0, dbmNormal);
        drawNumber_Right(x, 45, hand.getLastHandScore().upgradeBones);
        x = x - Constants::HandScore_LineSpacing;
    }



    // Total Bones ..

    x = x - Constants::HandScore_LineSpacing + 4;
    drawNumber_Right(x, 45, hand.getLastHandScore().totalBones);
    arduboy.drawFastVLine(x + 7, 50, 11, WHITE);
    x = x - Constants::HandScore_LineSpacing - 5;
    FX::drawBitmap(x, 4, Images::Multiplier, 0, dbmNormal);
    x = x - Constants::HandScore_LineSpacing - 2;


    // Hand multiplier ..

    if (hand.getLastHandScore().handType != HandType::None) {

        FX::drawBitmap(x, 4, Images::Hand_Names, static_cast<uint8_t>(hand.getLastHandScore().handType), dbmNormal);
        drawNumber_Right(x, 45, hand.getLastHandScore().handMultiplier);
        x = x - Constants::HandScore_LineSpacing;
    
    }


    // Skull multiplier ..

    for (uint8_t i = 0; i < hand.getDeckCount(); i++) {

        if (hand.getDeckEntry(i).getMultiplier() > 0) {
            FX::drawBitmap(x - 1, 4, Images::Skull_Names, static_cast<uint8_t>(hand.getDeckEntry(i).getSkullType()), dbmNormal);
            drawNumber_Right(x, 45, hand.getDeckEntry(i).getMultiplier());
            x = x - Constants::HandScore_LineSpacing;
        }

    }


    // Upgrade multiplier ..

    if (hand.getLastHandScore().upgradeMultiplier > 0) {
        FX::drawBitmap(x, 4, Images::Upgrade, 0, dbmNormal);
        drawNumber_Right(x, 45, hand.getLastHandScore().upgradeMultiplier);
        x = x - Constants::HandScore_LineSpacing;

    }


    // Total multiplier ..

    x = x - Constants::HandScore_LineSpacing + 4;
    drawNumber_Right(x, 45, hand.getLastHandScore().totalMultiplier);
    arduboy.drawFastVLine(x + 7, 50, 11, WHITE);
    x = x - Constants::HandScore_LineSpacing;


    // Total ..

    x = x - Constants::HandScore_LineSpacing;
    FX::drawBitmap(x, 4, Images::Total, 0, dbmNormal);
    drawNumber_Right(x, 45, hand.getLastHandScore().score);
    arduboy.drawFastVLine(x + 7, 44, 17, WHITE);
    x = x - Constants::HandScore_LineSpacing - Constants::HandScore_LineSpacing - 5;


    handScore_xMax = handScore_x - x;
    
    HandScore_BottomOptions bottomOptions_Idx = HandScore_BottomOptions::None;

    if (handScore_x == Constants::HandScore_StartingPos) {

        if (x > 16) {
            bottomOptions_Idx = HandScore_BottomOptions::None;  
        }
        else {
            bottomOptions_Idx = HandScore_BottomOptions::DownOnly;
        }

    }
    else {

        if (x >= 0) {
            bottomOptions_Idx = HandScore_BottomOptions::UpOnly;
        }
        else {
            bottomOptions_Idx = HandScore_BottomOptions::Both;
        }

    }


    FX::drawBitmap(128 - 48, 0, Images::HandScore_Top, messageIdx, dbmNormal);
    FX::drawBitmap(79, -5, Images::Skull_Large, skullData.getMouth(), dbmWhite);
    if (skullData.getEyes() != 255) FX::drawBitmap(102, 4, Images::Skull_Eyes, skullData.getEyes(), dbmWhite);    
    FX::drawBitmap(0, 0, Images::HandScore_Bot, static_cast<uint8_t>(bottomOptions_Idx), dbmNormal);

}