#pragma once

#include "../utils/Arduboy2Ext.h"
#include <FixedPoints.h>
#include <FixedPointsCommon.h>

using FP8 = SFixed<8, 8>;

struct RadarEnemy {

    private:
        FP8      x, z;     // world position
        uint8_t hp;        // hit points (0 = destroyed)
        bool    active;    // false = skip rendering

    public:
        FP8 getX()                      { return this->x; }
        FP8 getZ()                      { return this->z; }
        uint8_t getHP()                 { return this->hp; }
        bool isActive()                 { return this->active; }

        void setX(FP8 val)              { this->x = val; }
        void setZ(FP8 val)              { this->z = val; }
        void setHP(uint8_t val)         { this->hp = val; }
        void setActive(bool val)        { this->active = val; }

};
