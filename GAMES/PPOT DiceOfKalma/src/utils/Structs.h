#pragma once

#include "Constants.h"

struct HandScore {

    uint16_t baseBones = 0;
    uint16_t handBones = 0;
    uint16_t skullBones = 0;
    uint16_t upgradeBones = 0;
    uint16_t totalBones = 0;
    uint8_t  handMultiplier = 0;
    uint8_t  skullMultiplier = 0;
    uint8_t  upgradeMultiplier = 0;
    uint8_t  totalMultiplier = 0;
    int16_t score = 0;

    HandType  handType = HandType::None;

    void reset() {

        this->baseBones = 0;
        this->handBones = 0;
        this->skullBones = 0;
        this->upgradeBones = 0;
        this->totalBones = 0;
        this->handMultiplier = 0;
        this->skullMultiplier = 0;
        this->upgradeMultiplier = 0;
        this->totalMultiplier = 0;
        this->score = 0;
        this->handType = HandType::None;

    }

};
