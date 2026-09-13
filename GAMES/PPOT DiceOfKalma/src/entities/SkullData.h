
#pragma once

#include <Arduino.h>
#include "../utils/Constants.h"
#include "../utils/Structs.h"

class SkullData {
    
    private:
        uint8_t eyes = 255;
        uint8_t mouth = 255;

    public:

        uint16_t getData()                  { return this->mouth * 256 + this->eyes; }

        void setData(uint16_t data)         { 

            this->eyes = data & 0x00FF;

            if ((data & 0xFF00) == 0x0100) {
                this->mouth = 0;
            }
            else if ((data & 0xFF00) == 0x0200) {
                this->mouth = 255;
            }
            else {

                if (this->mouth != 255) {
                    this->mouth++;
                }

            }

        }

        uint8_t getEyes() {

            return eyes;

        }

        uint8_t getMouth() {

            if (this->mouth == 255) return 0;

            switch (this->mouth % 4) {

                case 0:
                    return 0;

                case 1:
                case 3:
                    return 1;

                case 2:
                    return 2;
            }

            return 0;

        }

};
