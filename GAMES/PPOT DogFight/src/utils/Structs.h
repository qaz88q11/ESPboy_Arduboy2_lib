#pragma once

#include "Arduboy2Ext.h"
#include <FixedPoints.h>
#include <FixedPointsCommon.h>

using FP8 = SFixed<8, 8>;
using FP16 = SFixed<16, 16>;

struct HorizonCloudPos { 
    uint8_t x, y; 
};

struct EnemySpriteEntry {
    const uint8_t* fwdImg;
    const uint8_t* bckImg;
    const uint8_t* mask;
    int8_t offX;   // subtract from sx
    int8_t offY;   // subtract from sy
};


// 3-component fixed-point vector (Y unused for flat-plane flight) ..

struct Vec3 {
    FP8 x, y, z;
};



// Projected screen point ..

struct ScreenPoint {
    int8_t  x, y;               // screen pixel coords
    uint8_t size;               // apparent radius in pixels
    bool    valid;              // false if behind camera or off-screen
};


// Horizon state ..

struct HorizonState {
    FP8 tiltAngle = FP8(0);     // signed bank tilt degrees: +ve = right turn
    FP8 dipOffset = FP8(0);     // always >= 0: downward shift in pixels from turn G-load
};



