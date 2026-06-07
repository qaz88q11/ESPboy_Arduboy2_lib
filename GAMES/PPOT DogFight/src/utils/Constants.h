

#pragma once

#include "Arduboy2Ext.h"
#include <FixedPoints.h>
#include <FixedPointsCommon.h>
#include "Structs.h"
#include "Constants.h"

#define xDEBUG
#define xDEBUG_BREAK    asm volatile("break\n");

using FP8 = SFixed<8, 8>;   // Q8.8: range ±127.996, precision 0.00390625
using FP16 = SFixed<16, 16>;

namespace Constants {

    // General

    constexpr FP8 Furtherest_Distance           = FP8(80.0);                            // objects farther are 'out of the game'
    constexpr FP8 Closest_Distance              = FP8(0.5);                             // objects closer are culled
    constexpr int16_t Radar_Range               = 80;

    constexpr FP8   Altitude_Min                = FP8(-5.0);
    constexpr FP8   Altitude_Max                = FP8(5.0);

    constexpr uint8_t  Screen_W                 = 128;
    constexpr uint8_t  Screen_H                 = 64;
    constexpr uint8_t  Screen_Half_W            = Screen_W / 2;   // 64
    constexpr uint8_t  Screen_Half_H            = Screen_H / 2;   // 32
    constexpr FP8      FOV_Half_W               = FP8(Constants::Screen_Half_W);        // used in perspective divide

    // Cloud X = 5 + i*25 (uniform spacing), only Y stored.
    constexpr uint8_t CloudY[] PROGMEM          = { 5, 18, 9, 24, 4, 14, 28, 8, 22, 13 };

    constexpr uint8_t Cloud_Count               = 10;



    // Enemy tuning ..

    constexpr uint8_t Enemy_Count               = 5;
    constexpr uint8_t Heart_Idx                 = Enemy_Count;                          // index of heart pickup in enemies[]
    constexpr uint8_t Entity_Count              = Enemy_Count + 1;                      // enemies + heart pickup
    constexpr int16_t Enemy_Spawn_Dist          = Radar_Range + 15;                     // spawn this far out (> radar)

    constexpr FP8 Enemy_Speeds[3]               = { FP8(0.15), FP8(0.20), FP8(0.35) };  // Different speeds so they are going slower than the player most times ..
    constexpr FP8 Enemy_Turn_Rate               = FP8(0.75);                            // Degrees / frame normal steering

    constexpr int16_t Enemy_Spawn_Dist2         = 110 * 110;                            // = 12100 — well beyond spawn
    constexpr uint8_t Enemy_Respawn_Frames      = 64;                                   // ~2 s dead before respawning
    constexpr uint8_t Heart_Respawn_Frames      = 180;                                  // ~6 s before heart pickup reappears

    constexpr int16_t Enemy_Shot_Range2         = 50 * 50;                              // max squared distance to fire
    constexpr uint8_t Enemy_Shot_Cooldown       = 10;                                   // frames between shots (~3 s at 30 fps)
    constexpr uint8_t Enemy_Shot_Cooldown_Ext   = 90;                                   // frames between shots (~3 s at 30 fps)
    constexpr uint8_t Enemy_Shot_Cone           = 15;                                   // ±degrees alignment required to fire
    constexpr uint8_t Enemy_Shot_Hit_Chance     = 3;                                    // 1-in-N chance of a shot hitting


    // Speed factor — scales enemy movement, increases with each kill ..

    constexpr FP8 SpeedFactor_Init              = FP8(0.3);                             // Starting multiplier
    constexpr FP8 SpeedFactor_Inc               = FP8(0.10);                            // Added per kill
    constexpr FP8 SpeedFactor_Max               = FP8(2.0);                             // Hard cap


    // Player tuning ..

    constexpr FP8 Cruise_Speed                  = FP8(0.20);                            // Always-on baseline airspeed
    constexpr FP8 Boost_Speed_Max               = FP8(0.40);                            // Max speed when A held
    constexpr FP8 Boost_Acc                     = FP8(0.05);                            // Acceleration into boost (per frame)
    constexpr FP8 Boost_Drag                    = FP8(0.94);                            // Speed decay when A released (approaches cruise, not zero)

    constexpr FP8 Pitch_Rate_Acc                = FP8(0.60);
    constexpr FP8 Pitch_Rate_Drag               = FP8(0.78);
    constexpr FP8 Pitch_Rate_Max                = FP8(3.0);
    constexpr FP8 Pitch_Gravity                 = FP8(0.97);                            // Nose returns toward level
    constexpr FP8 Pitch_Sink_Rate               = FP8(0.04);                            // Passive sink (overcome by pitch up)
    constexpr FP8 Pitch_PX_Per_Deg              = FP8(1.25);                            // Screen pixels per pitch degree (1.2 → nearest Q8.8)

    constexpr FP8 Yaw_Rate_Acc                  = FP8(0.75);
    constexpr FP8 Yaw_Rate_Drag                 = FP8(0.80);
    constexpr FP8 Yaw_Rate_Max                  = FP8(4.5);

    constexpr FP8 Bank_Deg_Max                  = FP8(40.0);
    
    constexpr FP8 Bank_Slew_Rate                = FP8(0.14);                            // How fast tilt chases target   (~36/256)
    constexpr FP8 Dip_Slew_Rate                 = FP8(0.18);                            // How fast dip chases target    (~46/256)
    constexpr FP8 Max_Dip_PX                    = FP8(14);                              // Max downward shift on hard turns (pixels)


    // Radar geometry ..

    constexpr uint8_t Radar_Box_W               = 26;
    constexpr uint8_t Radar_Box_H               = 26;
    constexpr uint8_t Radar_Left                = 102;
    constexpr uint8_t Radar_Top                 = Screen_H - Radar_Box_H;  
    constexpr uint8_t Radar_Centre_X            = Radar_Left + Radar_Box_W / 2;  
    constexpr uint8_t Radar_Centre_Y            = Radar_Top + Radar_Box_H / 2; 
    constexpr uint8_t Radar_Radius              = 12;


    // Title constants

    constexpr uint8_t Drop_Letter_Count         = 8; 
    constexpr uint8_t Drop_Letter_H             = 24; 
    constexpr uint8_t Drop_Ground_Y2            = 40;                                   // half-px → pixel 20, centres 24px letter at y=32
    constexpr int8_t  Drop_Start_Y2             = -48;                                  // half-px → pixel -24, just above screen
    constexpr uint8_t Drop_Gravity              = 2; 
    constexpr uint8_t Drop_Restitution_N        = 6; 
    constexpr uint8_t Drop_Restitution_D        = 10; 
    constexpr uint8_t Drop_Settle_Vel           = 3;                                    // settle when upward speed < 1.5 px/frame
    constexpr uint8_t Drop_Stagger              = 8;                                    //  frames between each letter

}


// ── Enum ─────────────────────────────────────────────────────────────────

enum class AIState : uint8_t {
    Inbound,  // flying toward (and past) the player
    Turning,  // turning back to face the player after passing
    Explode,
    Dead,
};

enum class GameState : uint8_t {
    SplashScreen,
    Title_Init,
    Title,
    Playing,
    GameOver_Init,
    GameOver_Drop,
    GameOver
};