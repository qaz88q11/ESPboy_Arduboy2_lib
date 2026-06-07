#pragma once

#include "Arduboy2Ext.h"
#include "Maths.h"



// ── updateHorizonState ────────────────────────────────────────────────────
// tiltAngle — linear signed bank: left turn tilts left, right turn tilts right.
// dipOffset — always-positive downward shift driven by |yawRate|² plus |pitchRate|,
//             so the dip grows the harder the player turns or pitches.
void updateHorizonState(HorizonState& hs, const Player& player) {
    // norm = yawRate / Constants::Yaw_Rate_Max, range -1..+1 in Q8.8
    FP8 norm = player.getYawRate() / Constants::Yaw_Rate_Max;

    // Bank tilt: linear, signed degrees
    FP8 targetTilt = norm * Constants::Bank_Deg_Max;
    hs.tiltAngle += (targetTilt - hs.tiltAngle) * Constants::Bank_Slew_Rate;

    // Dip: quadratic — norm*norm is always 0..+1 so dip is always downward.
    // norm*norm in Q8.8: multiply gives Q16.16 internally, assigned back to Q8.8 (fine for 0..1).
    FP8 yawDip   = norm * norm;
    // pitchNorm: pitchRate / Constants::Pitch_Rate_Max, range -1..+1
    FP8 pitchNorm = player.getPitchRate() / Constants::Pitch_Rate_Max;
    FP8 pitchDip  = pitchNorm * pitchNorm * FP8(0.4);  // 0..0.4
    FP8 targetDip = (yawDip + pitchDip) * Constants::Max_Dip_PX;
    hs.dipOffset += (targetDip - hs.dipOffset) * Constants::Dip_Slew_Rate;
}

// ── horizonYatX ──────────────────────────────────────────────────────────
// Returns the screen Y of the horizon line at a given screen X column.
//
//   pitchDeg  — nose pitch: +ve nose up → horizon moves down
//   tiltAngle — bank: signed, tilts the horizon line left or right
//   dipOffset — turn G-load: always >= 0, shifts the whole horizon down
//
// The tilt is a slope: left edge and right edge of the horizon move in
// opposite directions. The dip shifts the whole line equally downward.
static inline int16_t horizonYatX(uint8_t screenX,
                                   FP8 pitchDeg, FP8 tiltAngle,
                                   FP8 dipOffset = FP8(0)) {

    FP8 pitchShift = pitchDeg * Constants::Pitch_PX_Per_Deg;
    // Convert tiltAngle (signed FP8 degrees) to uint16_t for trig table
    int16_t  tiltInt = (int16_t)tiltAngle;
    uint16_t tiltU   = (uint16_t)((tiltInt % 360 + 360) % 360);
    FP8 sinT      = fpSin(tiltU);
    // (Constants::Screen_Half_W - screenX) is signed int8; multiply by FP8 sin value
    FP8 tiltShift = FP8((int8_t)((int8_t)Constants::Screen_Half_W - (int8_t)screenX)) * sinT;
    return (int16_t)(FP8(Constants::Screen_Half_H) + pitchShift + tiltShift + dipOffset);

}

// Rows 0-9 stored; rows 10-14 are computed: val = (row-8) + (col-1)*4
static const uint8_t PROGMEM horizon[10][9] = {
    { 0, 2,  4, 6, 8, 10, 12, 14, 16 },
    { 0, 2,  5, 6, 9, 10, 13, 15, 17 },
    { 0, 3,  5, 7, 9, 11, 13, 15, 17 },
    { 0, 3,  6, 7, 10, 11, 14, 15, 17 },
    { 0, 4, 6, 8, 10, 12, 14, 16, 18 },

    { 0, 2, 5, 8, 11, 14, 17, 20, 23 },
    { 0, 3, 6, 9, 12, 15, 18, 21, 24 },
    { 0, 3, 6,10, 13, 15, 19, 21, 25 },
    { 0, 4, 7,10, 13, 16, 19, 22, 26 },
    { 0, 5, 8,11, 14, 17, 20, 23, 26 },
};

static inline uint8_t horizonVal(uint8_t row, uint8_t col) {
    if (row < 10) return pgm_read_byte(&horizon[row][col]);
    if (col == 0) return 0;
    return (uint8_t)((row - 8) + (col - 1) * 4);
}

void drawHorizon(Arduboy2Ext& a, const Player& player, const HorizonState& hs, const GameState gameState) {

    FP8 pitchDeg  = player.getPitch();
    FP8 tiltAngle = hs.tiltAngle;
    FP8 dipOffset = hs.dipOffset;

    // Compute horizon Y at left and right edges ONCE (2 fpSin calls total).
    // Then step linearly across columns — no trig per column.
    // hy_left = horizonYatX(0, ...), hy_right = horizonYatX(127, ...)
    // slope_num/slope_den kept as integers: add slope_num/128 per column.
    int16_t hy_left  = horizonYatX(0,          pitchDeg, tiltAngle, dipOffset);
    int16_t hy_right = horizonYatX(Constants::Screen_W-1, pitchDeg, tiltAngle, dipOffset);

    // Integer slope stepping: hy increments by (hy_right - hy_left) / 127 per column.
    // Use fixed-point accumulator: slopeAccum in units of 1/128 pixel.
    // Multiply by 128 to avoid division inside the loop.
    int16_t slopeFrac  = (int16_t)((hy_right - hy_left) * 128) / (int16_t)(Constants::Screen_W - 1);
    int16_t hyAccum    = (int16_t)(hy_left * 128);

    uint8_t speed = player.getVelFwd().getFraction(); // gives a range of 88 to 128.
    uint8_t yData = 0;
    
    if (gameState == GameState::Playing) yData = ((a.frameCount / 2) % 5) + (speed > 90 ? 5 : 0) + (speed > 110 ? 5 : 0);

    for (uint8_t x = 0; x < Constants::Screen_W; x++) {

        int16_t hy = hyAccum / 128;
        hyAccum += slopeFrac;
        hy = constrain(hy, 0, Constants::Screen_H);

        // Ground — solid top edge row
        a.drawPixel(x, (int8_t)(hy + 1), WHITE);


        uint8_t xData = 1;
        for (int8_t y = (int8_t)(hy + 1 + horizonVal(yData, xData)); y < Constants::Screen_H; y += horizonVal(yData, xData)) {

            if (x % (3 + xData) == 0) {
                a.drawPixel(x, y, WHITE);
            }

            xData++;

        }

    }

}
