#pragma once

#include "../utils/Arduboy2Ext.h"
#include <FixedPoints.h>
#include <FixedPointsCommon.h>
#include "../utils/Constants.h"

using FP8 = SFixed<8, 8>;
using FP16 = SFixed<16, 16>;

class Player {

    private:

        #ifdef DEBUG
        int16_t  ix, iz;    // world position
        #endif

        FP8  x, z;          // world position
        FP8  y;             // altitude
        FP16 yaw;           // heading, 0-359 degrees
        FP8  pitch;         // nose angle: +ve up, -ve down (degrees)
        FP8  velFwd;        // current airspeed
        FP8  velY;          // vertical velocity (computed each frame)
        FP8  pitchRate;
        FP8  yawRate;
        uint8_t hp;
        uint16_t score = 0;

        // Temporary / per frame values ..

        uint8_t pressed = 0;
        uint8_t justPressed = 0;
        FP8 cosY;
        FP8 sinY;

        bool controlsInverted = false;      // Inverted means up goes up!

    public:

        #ifdef DEBUG
        void setX(FP8 val)                  { this->x = val; this->ix = val.getInteger(); }
        void setZ(FP8 val)                  { this->z = val; this->iz = val.getInteger(); }
        #else
        void setX(FP8 val)                  { this->x = val; }
        void setZ(FP8 val)                  { this->z = val; }
        #endif

        void setY(FP8 val)                  { this->y = val; }
        void setYaw(FP16 val)               { this->yaw = val; }
        void setPitch(FP8 val)              { this->pitch = val; }
        void setVelFwd(FP8 val)             { this->velFwd = val; }
        void setVelY(FP8 val)               { this->velY = val; }
        void setPitchRate(FP8 val)          { this->pitchRate = val; }
        void setYawRate(FP8 val)            { this->yawRate = val; }
        void setPressed(uint8_t val)        { this->pressed = val; }
        void setJustPressed(uint8_t val)    { this->justPressed = val; }
        void setHP(uint8_t val)             { this->hp = val; }
        void setCosY(FP8 val)               { this->cosY = val; }
        void setSinY(FP8 val)               { this->sinY = val; }
        void setScore(uint16_t val)         { this->score = val; }

        FP8 getX() const                         { return this->x; }
        FP8 getZ() const                         { return this->z; }
        FP8 getY() const                         { return this->y; }
        FP8 getPitch() const                     { return this->pitch; }
        FP8 getVelFwd() const                    { return this->velFwd; }
        FP8 getVelY() const                      { return this->velY; }
        FP8 getPitchRate() const                 { return this->pitchRate; }
        FP8 getYawRate() const                   { return this->yawRate; }
        FP8 getCosY() const                      { return this->cosY; }
        FP8 getSinY() const                      { return this->sinY; }
        FP16 getYaw() const                      { return this->yaw; }
        uint8_t getHP()                     { return this->hp; }
        uint16_t getScore()                 { return this->score; }

        bool boost()                        { return this->pressed & A_BUTTON; }
        bool pitchUp()                      { return this->pressed & (controlsInverted ? UP_BUTTON : DOWN_BUTTON); }
        bool pitchDown()                    { return this->pressed & (controlsInverted ? DOWN_BUTTON : UP_BUTTON); }
        bool yawLeft()                      { return this->pressed & LEFT_BUTTON; }
        bool yawRight()                     { return this->pressed & RIGHT_BUTTON; }
        bool fire()                         { return this->justPressed & B_BUTTON; }

        void decHP(uint8_t val) {
        
            if (this->hp >= val) {
            
                this->hp = this->hp - val;

            }
            else {

                this->hp = 0;
                
            }
        
        }

        void reset(bool invertControls) {

            this->x             = FP8(0);
            this->z             = FP8(0);
            this->y             = FP8(20);
            this->yaw           = FP16(0);
            this->pitch         = FP8(0);
            this->velFwd        = Constants::Cruise_Speed;
            this->velY          = FP8(0);
            this->pitchRate     = FP8(0);
            this->yawRate       = FP8(0);
            this->hp            = 15;
            this->pressed       = 0;
            this->justPressed   = 0;
            this->score         = 0;
            
            this->controlsInverted = invertControls;

        }

};
