// 3d_engine.h — Arduboy Fixed-Point 3D Engine
// Requires: Pharap's FixedPointsArduino library
//   https://github.com/Pharap/FixedPointsArduino
//
// Coordinate system:
//   +X = right,  +Y = up (unused, terrain is flat),  +Z = forward
//   Player always at origin in camera space.
//   Yaw rotates CCW when viewed from above (0° = look along -Z).

#pragma once

#include "Arduboy2Ext.h"
#include <FixedPoints.h>
#include <FixedPointsCommon.h>
#include "../entities/Entities.h"


using FP8 = SFixed<8, 8>;


// ── Quarter-wave tables (0°..90°, 19 entries each) ───────────────────────
// fpCos/fpSin fold all 360° into the first quadrant via symmetry, saving
// ~212 bytes vs the original full 72-entry COS_TABLE + SIN_TABLE.

static const int16_t COS_QW[19] PROGMEM = {
    256, 255, 252, 247, 240, 232, 222, 210,
    196, 181, 165, 147, 128, 108,  88,  66,
     44,  22,   0
};

static const int16_t SIN_QW[19] PROGMEM = {
      0,  22,  44,  66,  88, 108, 128, 147,
    165, 181, 196, 210, 222, 232, 240, 247,
    252, 255, 256
};

// Helper: interpolate a quarter-wave table at 0..90 degrees.
static inline int16_t qwLerp(const int16_t* tbl, uint16_t deg) {
    uint8_t base = (uint8_t)(deg / 5);
    uint8_t frac = (uint8_t)(deg % 5);
    uint8_t nxt  = (base < 18) ? base + 1 : 18;
    int16_t lo   = (int16_t)pgm_read_dword(&tbl[base]);
    int16_t hi   = (int16_t)pgm_read_dword(&tbl[nxt]);
    return lo + (int16_t)((int16_t)(hi - lo) * frac / 5);
}

// COS lookup — interpolated, 4-quadrant symmetry from COS_QW[19].
FP8 fpCos(uint16_t degrees) {
    degrees = degrees % 360;
    bool negate = false;
    if      (degrees <= 90)  { /* Q1: direct */                            }
    else if (degrees <= 180) { degrees = 180 - degrees; negate = true;     }
    else if (degrees <= 270) { degrees = degrees - 180; negate = true;     }
    else                     { degrees = 360 - degrees;                    }
    int16_t raw = qwLerp(COS_QW, degrees);
    return FP8::fromInternal(negate ? -raw : raw);
}

// SIN lookup — interpolated, 4-quadrant symmetry from SIN_QW[19].
FP8 fpSin(uint16_t degrees) {
    degrees = degrees % 360;
    bool negate = false;
    if (degrees >= 180) { negate = true; degrees -= 180; }
    if (degrees > 90)   { degrees = 180 - degrees;       }
    int16_t raw = qwLerp(SIN_QW, degrees);
    return FP8::fromInternal(negate ? -raw : raw);
}

// ── 1/z lookup: near-range table + computed far range ────────────────────
// INV_Z_NEAR covers indices 0-15 (z ≈ 0..5 world units) exactly.
// For z > 5 the formula 112640/z.getInternal() matches the original table
// to within ~4%, which is imperceptible on this 128×64 display.
// Saves ~480 bytes vs the original 256-entry INV_Z_LUT[256].

static const uint16_t INV_Z_NEAR[16] PROGMEM = {
    512, 512, 410, 315, 256, 219, 192, 171,
    154, 140, 128, 118, 110, 102,  96,  90
};

FP8 getInvZ(FP8 z) {
    if (z >= Constants::Furtherest_Distance) return FP8(0.0125); // 1/80
    uint16_t zi    = (uint16_t)z.getInternal();
    uint8_t  index = (uint8_t)(((uint32_t)zi * 32) / 2560);
    if (index < 16) return FP8::fromInternal(pgm_read_dword(&INV_Z_NEAR[index]));
    return FP8::fromInternal((uint16_t)(112640u / zi));
}


// ── worldToCameraFast ──────────────────────────────────────────────────────
// Use precomputed cosY/sinY (call fpCos/fpSin once per frame, reuse here).

Vec3 worldToCameraFast(FP8 dx, FP8 dz, FP8 cosY, FP8 sinY) {

    return Vec3 {
         dx * cosY + dz * sinY,
         FP8(0),
        -dx * sinY + dz * cosY
    };

}

// ── cameraCosSin ──────────────────────────────────────────────────────────
// Compute the cosY/sinY for the player's current yaw. Call once per frame
// and pass cosY/sinY to worldToCameraFast and drawRader.
void cameraCosSin(Player& player) {

    uint16_t inverseYaw = (360 - player.getYaw().getInteger()) % 360;

    player.setCosY(fpCos(inverseYaw));
    player.setSinY(fpSin(inverseYaw));

}



// Calculate the difference between two angles ..

static int16_t angleDiff(uint16_t from, uint16_t to) {
    int16_t d = to - from;
    while (d >  180) d -= 360;
    while (d < -180) d += 360;
    return d;
}


// Return the bearing between two points, (ax,az) to (bx,bz) ..

static uint16_t bearing(FP8 ax, FP8 az, FP8 bx, FP8 bz) {

    int32_t dx_raw = bx.getInternal() - ax.getInternal();
    int32_t dz_raw = bz.getInternal() - az.getInternal();
    int16_t dx = (int16_t)(dx_raw >> 4);
    int16_t dz = (int16_t)(dz_raw >> 4);

    if (dx == 0 && dz == 0) return 0;

    int16_t absX = (dx < 0) ? -dx : dx;
    int16_t absZ = (dz < 0) ? -dz : dz;
    uint16_t angle;

    if (absZ >= absX) {
        angle = (uint16_t)((absX * 45L) / absZ);
    }
    else {
        angle = 90 - (uint16_t)((absZ * 45L) / absX);
    }

    if (dz >= 0) {
        if (dx >= 0)        return angle;
        else                return 360 - angle;
    } 
    else {
        if (dx >= 0)        return 180 - angle;
        else                return 180 + angle;
    }

}


// Return the distance squared between two points - no use trying to take a square root to get actual distance as this is expensive ..

static int16_t distanceSquared(FP8 ax, FP8 az, FP8 bx, FP8 bz) {
    int16_t dx = (int16_t)((ax - bx).getInternal() >> 8);
    int16_t dz = (int16_t)((az - bz).getInternal() >> 8);
    return dx * dx + dz * dz;
}