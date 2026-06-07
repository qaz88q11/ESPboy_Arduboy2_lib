#pragma once

#include "../utils/Arduboy2Ext.h"
#include <FixedPoints.h>
#include <FixedPointsCommon.h>
#include "../utils/Constants.h"

using FP8 = SFixed<8, 8>;
using FP16 = SFixed<16, 16>;


// Enemy Plane ..

class EnemyPlane {

    private:

        #ifdef DEBUG
        int16_t  ix, iz, iy, iyaw;   // world position
        #endif

        FP8      x, z, y;                   // world position
        FP8      speed;                     // speed
        FP16     yaw;                       // heading (0-359) 0 = North / Up the radar, 90 = East, 180 = South, 270 = West
        uint8_t  hp;
        bool     active;                    // false = just destroyed (during DEAD countdown)
       
        AIState  state;
        uint16_t  timer;                    // Multipurpose countdown
        int8_t   arcDir;                    // +1 = arc right, -1 = arc left
               
        int8_t   passOffsetDeg;             // Per-enemy pass-offset: heading offset so each enemy flies a different line
        uint8_t  explosionFrame = 0;
        int16_t  d2Prev = 30000;            // Distance squared sampled 30 frames ago, for divergence detection
        uint8_t  shotCooldown = 0;
        uint8_t  muzzleFlashFrames = 0;
        bool     headingToward = false;

    public:

        #ifdef DEBUG
        void setX(FP8 val)                      { this->x = val; this->ix = val.getInteger(); }
        void setY(FP8 val)                      { this->y = val; this->iy = val.getInteger(); }
        void setZ(FP8 val)                      { this->z = val; this->iz = val.getInteger(); }
        void setYaw(FP16 val)                   { this->yaw = val; this->iyaw = val.getInteger(); }
        #else
        void setX(FP8 val)                      { this->x = val; }
        void setY(FP8 val)                      { this->y = val; }
        void setZ(FP8 val)                      { this->z = val; }
        void setYaw(FP16 val)                   { this->yaw = val; }
        #endif

        void setSpeed(FP8 val)                  { this->speed = val; }
        void setD2Prev(uint16_t val)            { this->d2Prev = val; }
        void setHP(uint8_t val)                 { this->hp = val; }        
        void setExplosionFrame(uint8_t val)     { this->explosionFrame = val; }        
        void incExplosionFrame()                { this->explosionFrame++; }        
        void setArcDir(int8_t val)              { this->arcDir = val; }        
        void setPassOffsetDeg(int8_t val)       { this->passOffsetDeg = val; }        
        void setTimer(uint16_t val)             { this->timer = val; }
        void setActive(bool val)                { this->active = val; }
        void setState(AIState val)              { this->state = val; }
        void setShotCooldown(uint8_t val)       { this->shotCooldown = val; }
        void setMuzzleFlashFrames(uint8_t val)  { this->muzzleFlashFrames = val; }

        FP8 getX()                              { return this->x; }
        FP8 getY()                              { return this->y; }
        FP8 getZ()                              { return this->z; }
        FP8 getSpeed()                          { return this->speed; }
        FP16 getYaw()                           { return this->yaw; }
        uint16_t getD2Prev()                    { return this->d2Prev; }
        uint8_t getHP()                         { return this->hp; }
        uint8_t getExplosionFrame()             { return this->explosionFrame; }        
        int8_t getArcDir()                      { return this->arcDir; }
        int8_t getPassOffsetDeg()               { return this->passOffsetDeg; }
        uint16_t getTimer()                     { return this->timer; }
        bool isActive()                         { return this->active; }
        AIState getState()                      { return this->state; }
        uint8_t getShotCooldown()               { return this->shotCooldown; }
        uint8_t getMuzzleFlashFrames()          { return this->muzzleFlashFrames; }
        bool getHeadingToward()                 { return this->headingToward; }
        void setHeadingToward(bool val)         { this->headingToward = val; }

        void incTimer() {
            this->timer++;
        }

};
