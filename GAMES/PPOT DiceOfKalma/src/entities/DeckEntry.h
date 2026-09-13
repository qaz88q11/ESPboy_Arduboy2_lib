#pragma once

#include <Arduino.h>

#include "../utils/Constants.h"
#include "../utils/Structs.h"

class DeckEntry {
    
    private:
    
        SkullType skullType;
        uint8_t bones;
        uint8_t multiplier;

    public:

        SkullType getSkullType()                { return this->skullType; }
        uint8_t getBones()                      { return this->bones; }
        uint8_t getMultiplier()                 { return this->multiplier; }

        void setSkullType(SkullType val)        { this->skullType = val; }
        void setBones(uint8_t val)              { this->bones = val; }
        void setMultiplier(uint8_t val)         { this->multiplier = val; }

};
