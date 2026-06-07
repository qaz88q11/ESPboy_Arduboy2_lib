#pragma once

#include <Arduboy2.h>
#include "../utils/Constants.h"
#include "../utils/Enums.h"

struct Puzzle {

    private:

        PuzzleStatus status = PuzzleStatus::Locked;
        uint16_t time;

    public:

        uint16_t getTime() const                        { return this->time; }
        PuzzleStatus getStatus() const                  { return this->status; }

        void setTime(uint16_t val)                      { this->time = val; }
        void setStatus(PuzzleStatus val)                { this->status = val; }

};