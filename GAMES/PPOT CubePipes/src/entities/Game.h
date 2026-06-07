#pragma once

#include <Arduboy2.h>
#include "../utils/Constants.h"
#include "../utils/Enums.h"
#include "Puzzle.h"

struct Game {

    public:

        uint8_t puzzle_Orig[57] ;
        uint8_t puzzle[57];
        uint8_t puzzle_Grey[57];

    private:

        uint8_t saveData[Constants::Undo_Count][57];
        uint8_t savePosition[Constants::Undo_Count];
        uint8_t level = 0;
        int8_t world_Y_Offset = 0;
        uint16_t frameCount = 0;
        uint16_t time = 0;
        uint8_t undoCount = 0;
        uint8_t cursor = 0;
        uint8_t savedCursor = 0;

        Puzzle puzzles[2][Constants::Level_Count];
        PuzzleSize puzzleSize = PuzzleSize::Small;

    public:

        uint16_t getFrameCount()                        { return this->frameCount; }
        uint16_t getFrameCount(uint8_t val)             { return this->frameCount % val < val / 2; }
        int8_t getWorld_Y_Offset()                      { return this->world_Y_Offset; }
        uint8_t getLevel()                              { return this->level; }
        uint8_t getUndoCount()                          { return this->undoCount; }
        uint16_t getTime()                              { return this->time; }
        uint16_t getCursor()                            { return this->cursor; }
        Puzzle &getPuzzle(uint8_t size, uint8_t level)  { return this->puzzles[size][level]; }
        PuzzleSize getPuzzleSize()                      { return this->puzzleSize; }
        uint8_t getSaveData(uint8_t pos)                { return this->saveData[this->undoCount][pos]; }

        void setTime(uint16_t val)                      { this->time = val; }
        void setFrameCount(uint16_t val)                { this->frameCount = val; }
        void setWorld_Y_Offset(uint8_t val)             { this->world_Y_Offset = val; }
        void setLevel(uint8_t val)                      { this->level = val; }
        void setCursor(uint8_t val)                     { this->cursor = val; }
        void setPuzzleSize(PuzzleSize val)              { this->puzzleSize = val; }

        uint8_t getNumberOfCubes()  {
        
            if (this->puzzleSize == PuzzleSize::Small) {
                return 21;
            }

            return 57;

        }

        void incCursor(int8_t val)  {
        
            this->cursor = this->cursor + val;

        }

        void resetLevel() {

            this->undoCount = 0;

        }

        void incFrameCount() {

            this->frameCount++;

        }

        void incTime() {

            this->time++;

        }

        void resetFrameCount() {

            this->frameCount = 0;
            
        }

        void captureMove() {

            if (this->cursor == this->savedCursor) return;

            if (this->undoCount == Constants::Undo_Count) {

                for (uint8_t i = 0; i < Constants::Undo_Count - 1; i++) {
                    memcpy(this->saveData[i], this->saveData[i + 1], this->getNumberOfCubes());
                }

                this->undoCount = 7;
            }
            
            for (uint8_t i = 0; i < this->getNumberOfCubes(); i++) {
                this->saveData[this->undoCount][i] = this->puzzle[i];
            }

            this->savePosition[this->undoCount] = this->getCursor();

            if (this->undoCount < Constants::Undo_Count) this->undoCount++;
            this->savedCursor = this->cursor;
  
        }

        void revertMove() {

            this->undoCount--;

            for (uint8_t i = 0; i < this->getNumberOfCubes(); i++) {
                this->puzzle[i] = this->saveData[this->undoCount][i];
            }

            this->cursor = this->savePosition[this->undoCount];

        }

        void loadMap(uint8_t level) {

            this->resetLevel();

        }

};