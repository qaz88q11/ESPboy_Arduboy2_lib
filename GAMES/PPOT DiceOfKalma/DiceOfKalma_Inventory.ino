#include "src/utils/Arduboy2Ext.h"
#include "src/utils/Utils.h"
#include "src/utils/Constants.h"

void setDeckViewTop() {

    switch (hand.getDeckCount()) {
    
        case 0 ... 4:
            deckViewTop = 0;
            break;

        case 5:
            deckViewTop = deckViewCursor == 4 ? 1 : 0;
            break;

        case 6:

            switch (deckViewCursor) {

                case 0 ... 2:
                    deckViewTop = 0;
                    break;

                case 3:
                    deckViewTop = 1;
                    break;

                default:
                    deckViewTop = 2;
                    break;

            }

            break;

    }

}

void deckView() {

    if (arduboy.justPressed(LEFT_BUTTON) && hand.getDeckCount() > 0 && deckViewCursor < hand.getDeckCount() - 1) {

        deckViewCursor++;
        setDeckViewTop();

    }

    if (arduboy.justPressed(RIGHT_BUTTON) && hand.getDeckCount() > 0 && deckViewCursor > 0) {

        deckViewCursor--;
        setDeckViewTop();

    }

    if (arduboy.justPressed(DOWN_BUTTON)) {
    
        skullInfoType = hand.getDeckEntry(deckViewCursor).getSkullType();
        gameState = GameState::Game_Skull_Info; 
        returnState = GameState::Game_Deck_View;

    }

    if (arduboy.justPressed(A_BUTTON)) {

        switch (gameState) {
        
            case GameState::Game_Deck_Full_Swap:
                
                gameState = GameState::Game_Upgrade_Choice_Init;

                if (arduboy.justPressed(A_BUTTON)) {
                    hand.getDeckEntry(deckViewCursor).setSkullType(pendingSkull);
                }

                deckViewCursor = 0;
                deckViewTop = 0;

                break;

            case GameState::Game_Deck_View:

                gameState = GameState::Game_Roll;
                deckViewCursor = 0;
                deckViewTop = 0;

                break;

        }

    }


    if (arduboy.justPressed(B_BUTTON) && gameState != GameState::Game_Deck_View) {

        gameState = GameState::Game_Upgrade_Choice_Init;

    }


    // Render screen ..

    uint8_t x = 90;
    Inventory_BottomOptions botImage = Inventory_BottomOptions::Both;


    switch (gameState) {
    
        case GameState::Game_Deck_Full_Swap:
            FX::drawBitmap(115, 0, Images::Swap, 0, dbmNormal);
            break;

        case GameState::Game_Deck_View:
            FX::drawBitmap(96, 0, Images::Inventory_Top, 0, dbmNormal);
            drawNumber(119, 44, hand.getDeckCount());
            break;

    }



    if (hand.getDeckCount() == 0) {
        botImage = Inventory_BottomOptions::NoSkulls;
    }
    else if (hand.getDeckCount() < 5) {
        botImage = Inventory_BottomOptions::NoScrolling;
    }
    else if (deckViewCursor > 0) { 

        if (deckViewCursor == hand.getDeckCount() -1) {
            botImage = Inventory_BottomOptions::UpOnly;
        }
        else {
            botImage = Inventory_BottomOptions::Both;
        }
        
    }
    else { 

        if (deckViewCursor == hand.getDeckCount() -1) {
            botImage = Inventory_BottomOptions::NoScrolling;

        }
        else {
            botImage = Inventory_BottomOptions::DownOnly;
        }

    }
    
    FX::drawBitmap(0, 0, Images::Inventory_Bot, static_cast<uint8_t>(botImage), dbmNormal);

    if (hand.getDeckCount() != 0) {

        for (uint8_t i = deckViewTop; i < hand.getDeckCount(); i++) {

            uint24_t aIcon = FX::readIndexedUInt24(Images::Skulls_Thumb, static_cast<uint8_t>(hand.getDeckEntry(i).getSkullType()));
            FX::drawBitmap(x, 0, aIcon, 0, dbmNormal);

            x= x - 24;

        }

        if (arduboy.frameCount % 24 < 12) {
            FX::drawBitmap(90 - ((deckViewCursor - deckViewTop) * 24), 0, Images::Skull_Thumb_Cursor, 0, dbmWhite);
        }

    }

}
