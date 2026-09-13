#pragma once

#include "../utils/Arduboy2Ext.h"
#include "../utils/Constants.h"

struct Cookie {

    private:

        uint8_t bestLevel;

    public:
    
        uint8_t getBestLevel()              { return this->bestLevel; }
        void setBestLevel(uint8_t val)      { this->bestLevel = val; }

};
