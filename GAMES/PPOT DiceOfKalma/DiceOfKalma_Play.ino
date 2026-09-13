#include <ArduboyFX.h>  
#include <avr/pgmspace.h>

// const uint16_t levelThresholds[20] PROGMEM = {
//     176, 263, 353, 451, 560, 683, 825, 989, 1178, 1397, 1649, 1937, 2265, 2638, 3058, 3529, 4056, 4640, 5287, 6000
// };

// const uint16_t levelThresholds[20] PROGMEM = {
//     142, 213, 286, 365, 454, 554, 668, 801, 954, 1131, 1336, 1569, 1834, 2137, 2477, 2858, 3285, 3758, 4282, 4860
// };

const uint16_t levelThresholds[20] PROGMEM = {
142, 194, 247, 305, 370, 443, 527, 625, 737, 867,
1017, 1188, 1382, 1604, 1853, 2133, 2446, 2792, 3177, 3600
};

uint16_t computeThreshold() {

    if (level < 1) level = 1;
    if (level > 20) level = 20;
    return pgm_read_word(&levelThresholds[level - 1]);

}

void startLevel(bool transition) {

    threshold = computeThreshold();
    handsMax = 3 + (hand.countSkull(SkullType::Extra_Hand) > 0 ? 1 : 0);
    handsLeft = handsMax;
    rerollsMax = 3 + hand.countSkull(SkullType::Extra_Reroll);
    rerollsLeft = rerollsMax;
    hand.setFirstHandOfLevel(true);
    hand.setRerollUsedThisHand(false);
    hand.resetDice();
    stateTimer = 0;

    if (transition) gameState = GameState::Game_Level_Intro;

}

void newHand() {

    rerollsLeft = rerollsMax;
    cursor = 0;
    hand.setRerollUsedThisHand(false);
    takeTooLong =0;

}


void updateRoll() {

    takeTooLong++;

    if (takeTooLong % 256 == 0) { 
    
        takeTooLong_Type++;

        switch (takeTooLong_Type % 8) {

            case 0:
                addRollEyes(); 
                break;

            case 1:
                addLookLeft(false);
                break;

            case 2:
                addLookUp(false);
                break;

            case 3:
                addLookDown(false);
                break;

            case 4:
                addLookRight(false);
                break;

            case 5:
                addDeathStare(false);
                break;

            case 6:
                addPinPrick();
                break;       
                
            case 7:
                addWideEyes();
                break;                

        }

    }

    if (arduboy.frameCount > 5) {

        hand.setRerollHighlight(Constants::RerollHighlight_None);

    }

    hand.evaluateHand(); // live preview of current dice

    if (arduboy.justPressed(UP_BUTTON)) {

        cursor = (cursor == 0) ? CURSOR_REROLL : cursor - 1;

    }

    if (arduboy.justPressed(LEFT_BUTTON)) {

        if (cursor == 0 || cursor == 1) cursor = CURSOR_PLAY;
        if (cursor == 2) cursor = CURSOR_DECK;
        if (cursor == 3 || cursor == 4) cursor = CURSOR_REROLL;

    }

    if (arduboy.justPressed(RIGHT_BUTTON)) {

        if (cursor == CURSOR_PLAY) cursor = 0;
        if (cursor == CURSOR_DECK) cursor = 2;
        if (cursor == CURSOR_REROLL) cursor = 4;

    }

    if (arduboy.justPressed(DOWN_BUTTON)) {

        cursor = (cursor == CURSOR_REROLL) ? 0 : cursor + 1;

    }

    if (arduboy.pressed(B_BUTTON)) {
        bCounter++;
    }

    if (arduboy.justReleased(B_BUTTON)) {

        if (bCounter < 16) {

            gameState = GameState::Game_Hand_Info_Init;

        }

        bCounter = 0;

    }

    if (arduboy.justPressed(A_BUTTON)) {

        if (cursor < 5) {

            if (hand.getDisplayType(cursor) == DisplayType::RollAgain) {
                hand.setDisplayType(cursor, DisplayType::Display);
            }
            else {
                hand.setDisplayType(cursor, DisplayType::RollAgain);
            }

        } 
        else if (cursor == CURSOR_REROLL) {

            bool anyDisplayType = false;
            arduboy.frameCount = 0;
            hand.setRerollHighlight(Constants::RerollHighlight_Minimum);
            
            for (uint8_t i = 0; i < 5; i++) if (hand.getDisplayType(i) == DisplayType::RollAgain) anyDisplayType = true;

            if (rerollsLeft > 0 && anyDisplayType) {

                rerollsLeft--;
                hand.setRerollUsedThisHand(true);
                gameState = GameState::Game_Roll_Dice;
                returnState = GameState::Game_Roll;
                rollDice_Counter = 1;

            }

        } 
        else if (cursor == CURSOR_PLAY) { 

            hand.setPlayHandHighlight(Constants::PlayHandlHighlight_Minimum);
            hand.evaluateHand();
            handsLeft--;
            hand.setFirstHandOfLevel(false);
            gameState = GameState::Game_Hand_Result_Init;
            stateTimer = 0;

        } 
        else if (cursor == CURSOR_DECK) {

            returnState = GameState::Game_Roll;
            deckViewCursor = 0;
            gameState = GameState::Game_Deck_View;

        }

    }

}


void renderRollDice() {

    if (rollDice_Counter == 6) {

        for (uint8_t i = 0; i < 5; i++) {
            
            if (hand.getDisplayType(i) == DisplayType::RollAgain) {
        
                hand.rerollDice(i);

            }

        }

        // SJH

        // for (uint8_t i = 0; i < 5; i++) {
        //     if (i < 3){
        //         hand.setDice(i, 6);
        //     }
        //     else {
        //         hand.setDice(i, 1);
        //     }
        // }

    }

    if (arduboy.frameCount % 2 == 0) rollDice_Counter++;

    drawSkull();
    drawLevelAndTarget(hand.getLastHandScore(), level, threshold);
    drawBonesMultTotal(tempHandScore);
    drawDice();
    drawFooterRoll();

    if (rollDice_Counter == 9) {

        gameState = returnState;
        rollDice_Counter = 0;

        hand.markAllCards(DisplayType::Display);
        hand.setRerollHighlight(Constants::RerollHighlight_None);

    }

}


void renderHandResult_DrawFrame(HandScore tempHandScore) {

    drawSkull();
    drawBonesMultTotal(tempHandScore);
    drawDice();
    drawFooterRoll();

};


void renderHandResult_DrawSpeechBubble(uint8_t imageIdx) {

    Sprites::drawOverwrite(60, 0, Images::Speech_Bubble, 0);
    FX::drawBitmap(59, 0, Images::Speech_Sml, imageIdx, dbmWhite);

};


void renderHandResult_Base() {

    switch (renderHandResult_Counter) {
    
        case 0:

            hand.markAllCards(DisplayType::Display);
            renderHandResult_Counter++;
            renderHandResult_Timer = 0;
            addLookLeft(true);

            [[fallthrough]]

        case 1 ... 10:
            
            hand.markAllCards(DisplayType::Fade);
            renderHandResult_DrawFrame(tempHandScore);
            renderHandResult_Counter++;

            break;

        case 11:

            renderHandResult_Timer++;
            hand.markAllCards(DisplayType::Display);

            if (arduboy.isFrameCount(2) && tempHandScore.totalBones < hand.getLastHandScore().baseBones) {
                tempHandScore.totalBones++;
                tempHandScore.totalMultiplier = 0;
                tempHandScore.score++;
            }
            
            renderHandResult_DrawFrame(tempHandScore);
            renderHandResult_DrawSpeechBubble(9);

            if (renderHandResult_Timer > 48 && tempHandScore.totalBones == hand.getLastHandScore().baseBones) {
                renderHandResult_Counter++;
            }

            break;

        case 12 ... 15:

            renderHandResult_Counter++;
            renderHandResult_DrawFrame(tempHandScore);

            if (renderHandResult_Counter == 15) {
                renderHandResult_Counter = 0;

                if (hand.getLastHandScore().handBones > 0) {

                    gameState = GameState::Game_Hand_Result_Hand;

                }
                else if (hand.getLastHandScore().skullBones > 0) {

                    gameState = GameState::Game_Hand_Result_Skulls_Played;

                }
                else if (hand.getLastHandScore().upgradeBones > 0) {

                    gameState = GameState::Game_Hand_Result_Upgrades_Played;

                }
                else {

                    gameState = GameState::Game_Hand_Result_Countdown;
                
                };


            }

            break;

    }


}


void renderHandResult_Hand() {

    switch (renderHandResult_Counter) {
    
        case 0:

            hand.markAllCards(DisplayType::Display);
            renderHandResult_Counter++;
            renderHandResult_Timer = 0;
            addLookDown(true);            
            [[fallthrough]]

        case 1 ... 10:

            hand.markAllCards(DisplayType::Fade);
            renderHandResult_DrawFrame(tempHandScore);
            renderHandResult_Counter++;

            if (renderHandResult_Counter == 11) {
                
                renderHandResult_Timer = 0;
                hand.markAllCards(DisplayType::Display);

                if (hand.getLastHandScore().handMultiplier > 0) {

                    tempHandScore.totalMultiplier++;
                
                }

            }

            break;

        case 11:

            renderHandResult_Timer++;
            hand.markWiningHand(hand.getLastHandScore());

            if (hand.getLastHandScore().handBones > 10 || (hand.getLastHandScore().handBones <= 10 && arduboy.isFrameCount(4))) {

                if (tempHandScore.totalBones < hand.getLastHandScore().baseBones + hand.getLastHandScore().handBones) {

                    tempHandScore.totalBones++;

                    if ((tempHandScore.totalBones - hand.getLastHandScore().baseBones) % (hand.getLastHandScore().handBones / hand.getLastHandScore().handMultiplier) == 0 &&
                         tempHandScore.totalMultiplier < hand.getLastHandScore().handMultiplier) {
                      
                        tempHandScore.totalMultiplier++;
                        
                    }

                }

            }

            if (tempHandScore.totalMultiplier > 0) {
                tempHandScore.score = tempHandScore.totalBones * tempHandScore.totalMultiplier;
            }
            else {
                tempHandScore.score = tempHandScore.totalBones;
            }
            
            renderHandResult_DrawFrame(tempHandScore);
            renderHandResult_DrawSpeechBubble(static_cast<uint8_t>(hand.getLastHandScore().handType));
            
            if (renderHandResult_Timer > 48 && tempHandScore.totalBones == hand.getLastHandScore().baseBones + hand.getLastHandScore().handBones) {
                renderHandResult_Counter++;
            }

            break;

        case 12 ... 15:

            hand.markAllCards(DisplayType::Display);
            renderHandResult_Counter++;
            renderHandResult_DrawFrame(tempHandScore);

            if (renderHandResult_Counter == 15) {

                renderHandResult_Counter = 0;

                if (hand.getLastHandScore().skullBones > 0 || hand.getLastHandScore().skullMultiplier > 0) {

                    gameState = GameState::Game_Hand_Result_Skulls_Played;

                }
                else if (hand.getLastHandScore().upgradeBones > 0 || hand.getLastHandScore().upgradeMultiplier > 0) {

                    gameState = GameState::Game_Hand_Result_Upgrades_Played;

                }
                else {

                    gameState = GameState::Game_Hand_Result_Countdown;
                
                };

            }

            break;

    }

}

void renderHandResult_SkullsPlayed() {

    switch (renderHandResult_Counter) {

        case 0:

            hand.markAllCards(DisplayType::Display);
            renderHandResult_Counter++;
            addLookLeftThenRight(true);
            [[fallthrough]]

        case 1 ... 10:

            hand.markAllCards(DisplayType::Fade);
            renderHandResult_DrawFrame(tempHandScore);
            renderHandResult_Counter++;

            if (renderHandResult_Counter == 11) {
                renderHandResult_Timer = 0;

                if (hand.getLastHandScore().skullBones > 0) {
                    if (hand.getLastHandScore().skullMultiplier > 0) {
                        tempHandScore.totalMultiplier++;
                    }
                }
                else {
                    renderHandResult_Counter = 12; // No skull bones -> multiplier-only phase
                }

            }

            break;

        case 11: // Player has skull bones and maybe multipliers

            hand.markAllCards(DisplayType::Display);
            renderHandResult_Timer++;

            if (hand.getLastHandScore().skullBones >= 10 ||
                (hand.getLastHandScore().skullBones < 10 && arduboy.isFrameCount(4))) {

                if (tempHandScore.totalBones < hand.getLastHandScore().baseBones + hand.getLastHandScore().handBones + hand.getLastHandScore().skullBones) {

                    tempHandScore.totalBones++;

                    if (hand.getLastHandScore().skullMultiplier > 0 && (tempHandScore.totalBones - hand.getLastHandScore().baseBones - hand.getLastHandScore().handBones) % (hand.getLastHandScore().skullMultiplier / hand.getLastHandScore().skullMultiplier) == 0 &&
                        tempHandScore.totalMultiplier < hand.getLastHandScore().handMultiplier + hand.getLastHandScore().skullMultiplier) {
                        tempHandScore.totalMultiplier++;
                    }

                }

            }

            tempHandScore.score = tempHandScore.totalMultiplier > 0
                ? tempHandScore.totalBones * tempHandScore.totalMultiplier
                : tempHandScore.totalBones;

            renderHandResult_DrawFrame(tempHandScore);
            renderHandResult_DrawSpeechBubble(10);

            if (renderHandResult_Timer > 48 &&
                tempHandScore.totalBones == hand.getLastHandScore().baseBones + hand.getLastHandScore().handBones + hand.getLastHandScore().skullBones) {
                renderHandResult_Counter = 13; // always skips case 12 from here
            }

            break;

        case 12: // Player only has skull multiplier (skullBones == 0)

            hand.markAllCards(DisplayType::Display);
            renderHandResult_Timer++;

            if (renderHandResult_Timer % 8 == 0 && tempHandScore.totalMultiplier < hand.getLastHandScore().handMultiplier + hand.getLastHandScore().skullMultiplier) {
                tempHandScore.totalMultiplier++;
                tempHandScore.score = tempHandScore.totalBones * tempHandScore.totalMultiplier;
            }

            renderHandResult_DrawFrame(tempHandScore);
            renderHandResult_DrawSpeechBubble(10);

            if (renderHandResult_Timer > 48 &&
                tempHandScore.totalMultiplier == hand.getLastHandScore().handMultiplier + hand.getLastHandScore().skullMultiplier) {
                renderHandResult_Counter++;
            }

            break;

        case 13:

            hand.markAllCards(DisplayType::Display);
            [[fallthrough]]

        case 14 ... 16:

            renderHandResult_DrawFrame(tempHandScore);
            renderHandResult_Counter++;

            if (renderHandResult_Counter == 15) {

                renderHandResult_Counter = 0;

                gameState = hand.getLastHandScore().upgradeBones > 0
                    ? GameState::Game_Hand_Result_Upgrades_Played
                    : GameState::Game_Hand_Result_Countdown;

            }

            break;

    }

}

void renderHandResult_UpgradesPlayed() {

    switch (renderHandResult_Counter) {
    
        case 0:
            hand.markAllCards(DisplayType::Display);
            renderHandResult_Counter++;
            addLookLeftThenRight(true);
            [[fallthrough]]

        case 1 ... 10:

            hand.markAllCards(DisplayType::Fade);
            renderHandResult_DrawFrame(tempHandScore);
            renderHandResult_Counter++;

            if (renderHandResult_Counter == 11) {
                            
                renderHandResult_Timer = 0;

                if (hand.getLastHandScore().upgradeMultiplier > 0) {

                    tempHandScore.totalMultiplier++;
                
                }

            }

            break;

        case 11:

            hand.markAllCards(DisplayType::Display);
            renderHandResult_Timer++;

            if (hand.getLastHandScore().upgradeBones >= 10 || (hand.getLastHandScore().upgradeBones < 10 && arduboy.isFrameCount(4))) {

                if (tempHandScore.totalBones < hand.getLastHandScore().baseBones + hand.getLastHandScore().handBones + hand.getLastHandScore().skullBones + hand.getLastHandScore().upgradeBones) {

                    tempHandScore.totalBones++;

                    if ((tempHandScore.totalBones - hand.getLastHandScore().baseBones - hand.getLastHandScore().handBones - hand.getLastHandScore().skullBones) % (hand.getLastHandScore().upgradeBones / hand.getLastHandScore().upgradeMultiplier) == 0 &&
                        tempHandScore.totalMultiplier < hand.getLastHandScore().handMultiplier + hand.getLastHandScore().skullMultiplier + hand.getLastHandScore().upgradeMultiplier) {

                        tempHandScore.totalMultiplier++;
                        
                    }

                }

            }

            if (tempHandScore.totalMultiplier > 0) {
                tempHandScore.score = tempHandScore.totalBones * tempHandScore.totalMultiplier;
            }
            else {
                tempHandScore.score = tempHandScore.totalBones;
            }
            
            if (renderHandResult_Timer > 48 && (tempHandScore.totalBones == hand.getLastHandScore().baseBones + hand.getLastHandScore().handBones + hand.getLastHandScore().skullBones + hand.getLastHandScore().upgradeBones)) {

                if (tempHandScore.totalMultiplier < hand.getLastHandScore().handMultiplier + hand.getLastHandScore().skullMultiplier + hand.getLastHandScore().upgradeMultiplier) {
                    tempHandScore.totalMultiplier++;                    
                }

                renderHandResult_Counter++;

            }
            
            renderHandResult_DrawFrame(tempHandScore);
            renderHandResult_DrawSpeechBubble(11);

            break;

        case 12 ... 15:

            hand.markAllCards(DisplayType::Display);
            renderHandResult_Counter++;
            renderHandResult_DrawFrame(tempHandScore);

            if (renderHandResult_Counter == 15) {
                renderHandResult_Counter = 0;
                gameState = GameState::Game_Hand_Result_Countdown;
            }

            break;

    }

}

void renderHandResult_Countdown() {

    switch (renderHandResult_Counter) {
    
        case 0:

            countdownDiv = min(tempHandScore.score, threshold) / 50;
            if (countdownDiv == 0) countdownDiv = 1;
            hand.markAllCards(DisplayType::Display);
            renderHandResult_Counter++;
            addWideEyes();

            if (tempHandScore.score < threshold && handsLeft == 0) {
                messageIdx = random(8, 10);
            }
            else {
                messageIdx = random(0, 8);
            }
            [[fallthrough]]

        case 1 ... 10:

            hand.markAllCards(DisplayType::Fade);
            drawSkull();
            drawLevelAndTarget(hand.getLastHandScore(), level, threshold);
            drawBonesMultTotal(tempHandScore);
            drawDice();
            drawFooterRoll();
            renderHandResult_Counter++;

            break;

        case 11:

            hand.markAllCards(DisplayType::Display);

            if (tempHandScore.score >= countdownDiv && threshold >= countdownDiv) {

                tempHandScore.score = tempHandScore.score - countdownDiv;

                if (threshold - countdownDiv >= thresholdMin) {
                    threshold = threshold - countdownDiv;
                }
                else {
                    threshold = thresholdMin;
                }

            }
            else if (tempHandScore.score >= countdownDiv && threshold < countdownDiv) {

                tempHandScore.totalBones = 0;
                tempHandScore.totalMultiplier = 0;
                tempHandScore.score = tempHandScore.score - countdownDiv;
                threshold = 0;
                renderHandResult_Counter++;

            }
            else if (tempHandScore.score < countdownDiv && threshold >= countdownDiv) {

                tempHandScore.totalBones = 0;
                tempHandScore.totalMultiplier = 0;
                tempHandScore.score = 0;
                renderHandResult_Counter++;

                if (threshold - countdownDiv >= thresholdMin) {
                    threshold = threshold - countdownDiv;
                }
                else {
                    threshold = thresholdMin;
                }

            }
            else if (tempHandScore.score < countdownDiv && threshold < countdownDiv) {
                tempHandScore.score = 0;
                tempHandScore.totalBones = 0;
                tempHandScore.totalMultiplier = 0;
                threshold = 0;
                renderHandResult_Counter++;
            }
            
            drawSkull();
            drawLevelAndTarget(hand.getLastHandScore(), level, threshold);
            drawBonesMultTotal(tempHandScore);
            drawDice();
            drawFooterRoll();
             
            break;

        case 12 ... 15:

            threshold = thresholdMin;
            hand.markAllCards(DisplayType::Display);

            if (renderHandResult_Counter < 15) {

                renderHandResult_Counter++;

                if (renderHandResult_Counter == 15) {
                    addLongTalk();
                }

            }

            drawSkull();
            drawLevelAndTarget(hand.getLastHandScore(), level, threshold);
            drawBonesMultTotal(tempHandScore);
            drawDice();
            drawFooterRoll();

            if (renderHandResult_Counter == 15) {

                hand.setRerollHighlight(Constants::RerollHighlight_None);
                hand.setPlayHandHighlight(Constants::PlayHandHighlight_None);

                if (threshold == 0) {
                    
                    FX::drawBitmap(42, 0, Images::Speech_Lrg, messageIdx, dbmNormal);

                    if (arduboy.isFrameCount(16)) launchParticles();

                    if (arduboy.justPressed(A_BUTTON)) {
                        renderHandResult_Counter = 0;
                        level++;

                        if (level > Constants::Level_Count) {
                            gameState = GameState::Game_Win; 
                        }
                        else {
                            gameState = GameState::Game_Skull_Choice_Init;
                        }

                    }

                }
                else {
                
                    renderHandResult_Counter++;

                    if (handsLeft == 0) {
                        renderHandResult_Counter++;
                    }

                }

            }

            break;

        case 16:

            drawSkull();
            drawBonesMultTotal(tempHandScore);
            drawDice();
            FX::drawBitmap(42, 0, Images::Speech_Lrg, messageIdx, dbmNormal);
            drawFooterRoll();

            if (arduboy.justPressed(A_BUTTON)) {

                hand.markAllCards(DisplayType::RollAgain);
                renderHandResult_Counter = 0;
                rollDice_Counter = 0;
                gameState = GameState::Game_Roll_Dice;
                returnState = GameState::Game_Roll;

            }

            break;

        case 17:

            drawSkull();
            drawBonesMultTotal(tempHandScore);
            drawDice();
            FX::drawBitmap(42, 0, Images::Speech_Lrg, messageIdx, dbmNormal);
            drawFooterRoll();

            if (arduboy.justPressed(A_BUTTON)) {

                gameState = GameState::Game_Over;

            }

            break;

    }

}

