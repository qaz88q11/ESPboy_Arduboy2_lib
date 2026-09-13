#include <ArduboyFX.h>

bool zoomOut = false;
uint8_t zoomIdx = 0;

void title() {

    if (arduboy.justPressed(A_BUTTON)) {

        #ifdef DEBUG_SKIP_INTRO
            zoomIdx = 147; 
        #else
            zoomIdx = 0; 
        #endif

        zoomOut = true;
        startLevel(false);
        level = 1;
        arduboy.setFrameRate(60);

    }

    FX::drawBitmap(0, 0, Images::Fire, arduboy.getFrameCount(96) / 6, dbmNormal);

    if (zoomOut) {

        if (zoomIdx > 50) {

            FX::drawBitmap(76, 12, Images::Skull_Large, 0, dbmNormal);
            FX::drawBitmap(0, 0, Images::LevelEntry, 0, dbmMasked);
            drawNumber_Padded(43, 40, level, 2);
            drawNumber_Padded(32, 45, threshold, 4);
            drawNumber_Padded(21, 40, handsMax, 2);
            drawNumber_Padded(10, 43, rerollsMax, 2);

            uint24_t idx2 = FX::readIndexedUInt24(Images::Zoom,  min(zoomIdx, 108));

            if (zoomIdx < 108) {
                FX::drawBitmap(0, 0, idx2, 0, dbmMasked);
            }
            else {
                FX::drawBitmap((-zoomIdx + 108) * 5 /3, 0, idx2, 0, dbmMasked);
            }

        } 
        else {

            FX::drawBitmap(1, 0, Images::Title_Options, 0, dbmMasked);
            FX::drawBitmap(24, 0, Images::Title_Text, 0, dbmMasked);
            uint24_t idx = FX::readIndexedUInt24(Images::Zoom, zoomIdx);
            FX::drawBitmap(0, 0, idx, 0, dbmMasked);

        }

        zoomIdx++;

    } 
    else {

        FX::drawBitmap(1, 0, Images::Title_Options, 0, dbmMasked);
        arduboy.fillRect(11, 51, 7, 5, BLACK);
        if (cookie.getBestLevel() > 10) arduboy.fillRect(11, 55, 7, 5, BLACK);
        drawNumber(12, 46, cookie.getBestLevel());
        FX::drawBitmap(76, 12, Images::Skull_Large, skullData.getMouth(), dbmNormal);
        if (skullData.getEyes() != 255) FX::drawBitmap(99, 21, Images::Skull_Eyes, skullData.getEyes(), dbmWhite);
        FX::drawBitmap(24, 0, Images::Title_Text, 0, dbmMasked);

    }

    if (zoomOut && zoomIdx == 148) {

        level = 1;
        zoomIdx = 0;
        zoomOut = false;
        hand.setDeckCount(0);
        hand.reset();

        #ifdef DEBUG_ADD_SKULLS

        hand.addSkullToDeck(SkullType::Pair_Multiplier);
        hand.addSkullToDeck(SkullType::Big_Multiplier);
        hand.addSkullToDeck(SkullType::Kind_3or4_Multiplier);
        hand.addSkullToDeck(SkullType::Flat_Bones);
        hand.addSkullToDeck(SkullType::Extra_Hand);
        hand.addSkullToDeck(SkullType::Extra_Hand);
        hand.setDeckCount(6);

        #endif

        startLevel(true);
        arduboy.setFrameRate(30);

    }

}
