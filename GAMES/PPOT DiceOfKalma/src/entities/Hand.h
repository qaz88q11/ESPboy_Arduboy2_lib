#pragma once

#include <Arduino.h>

#include "../utils/Constants.h"
#include "../utils/Structs.h"
#include "DeckEntry.h"

class Hand {
    
    private:

        uint8_t dice[5];
        uint8_t deckCount = 0;
        uint8_t rerollHighlight = Constants::RerollHighlight_None;
        uint8_t playHandHighlight = Constants::PlayHandHighlight_None;

        bool rerollUsedThisHand = false;
        bool firstHandOfLevel = true;

        DeckEntry deck[MAX_DECK];
        HandScore lastHandScore;
        DisplayType displayType[5];

        bool upgradeHandsToShow[static_cast<uint8_t>(HandType::Count) - 2];                 // indicates which hands can be selected when upgrading, will never used HadType::None or HighRoll.
        HandType upgradeHand = HandType::None;


    public:
        
        uint8_t getDice(uint8_t idx)                            { return this->dice[idx]; }
        uint8_t getDeckCount()                                  { return this->deckCount; }
        bool getRerollUsedThisHand()                            { return this->rerollUsedThisHand; return 0;}
        bool isFirstHandOfLevel()                               { return this->firstHandOfLevel; return 0;}
        bool getRerollHighlight()                               { return this->rerollHighlight; return 0;}
        bool getPlayHandHighlight()                             { return this->playHandHighlight; return 0;}

        DeckEntry &getDeckEntry(int8_t idx)                     { return this->deck[idx]; }
        HandType getUpgradeHand()                               { return this->upgradeHand; }
        HandScore &getLastHandScore()                           { return this->lastHandScore; }
        DisplayType getDisplayType(uint8_t idx)                 { return this->displayType[idx]; }

        void setDice(uint8_t idx, uint8_t val)                  { this->dice[idx] = val; }
        void setDeckCount(uint8_t val)                          { this->deckCount = val; }
        bool setRerollUsedThisHand(bool val)                    { this->rerollUsedThisHand = val; return 0;}
        bool setFirstHandOfLevel(bool val)                      { this->firstHandOfLevel = val; return 0;}
        bool setRerollHighlight(bool val)                       { this->rerollHighlight = val; return 0;}
        bool setPlayHandHighlight(bool val)                     { this->playHandHighlight = val; return 0;}

        void setUpgradeHand(HandType val)                       { this->upgradeHand = val; }
        void setDisplayType(uint8_t idx, DisplayType val)       { this->displayType[idx] = val; }

    private:

        // Returns the dice value (not index) of the nominated qty.  If dice are 2, 3, 3, 5, 1 and you call getDice_OfaKind(2, 0) returns a 3.
        
        uint8_t getDice_OfaKind(uint8_t number, uint8_t higherThan) { 

            uint8_t counts[7] = {0, 0, 0, 0, 0, 0, 0};
            for (uint8_t i = 0; i < 5; i++) counts[this->dice[i]]++;

            for (uint8_t i = higherThan; i < 7; i++) {
                if (counts[i] == number && i > higherThan) return i;
            }

            return 0;

        }

        uint8_t getFirstDice(uint8_t number) { 

            for (uint8_t i = 0; i < 6; i++) {
                if (this->dice[i] == number) return i;
            }

            return 255;

        }

    public:

        void reset() {

            this->deckCount = 0;
            this->clearDeck();

            for (uint8_t i = 0; i < 6; i++) {
                this->deck[i].setSkullType(SkullType::None);
            }

        }

        void markAllCards(DisplayType DisplayTypeVal){
        
            for (uint8_t i = 0; i < 5; i++) this->displayType[i] = DisplayTypeVal;

        }

        void clearDeck() {
        
            for (uint8_t i = 0; i < MAX_DECK; i++) {
                this->deck[i].setBones(0);
                this->deck[i].setMultiplier(0);
            }

        }

        void markWiningHand(HandScore handScore) {
        
            switch (handScore.handType) {
            
                case HandType::Five_of_a_Kind:
                case HandType::Full_House:
                case HandType::Straight:
                
                    this->markAllCards(DisplayType::Display);
                    break;
            
                case HandType::Four_of_a_Kind:
                    {
                        uint8_t diceValue = this->getDice_OfaKind(4, 0);
                        for (uint8_t i = 0; i < 5; i++) {
                            if (this->dice[i] != diceValue) {
                                this->displayType[i] = DisplayType::Hide;
                            }
                        }
                    }
                    break;
            
                case HandType::Three_of_a_Kind:
                    {
                        uint8_t diceValue = this->getDice_OfaKind(3, 0);
                        for (uint8_t i = 0; i < 5; i++) {
                            if (this->dice[i] != diceValue) {
                                this->displayType[i] = DisplayType::Hide;
                            }
                        }

                    }
                    break;

                case HandType::Two_Pair:
                    {

                        for (uint8_t i = 0; i < 5; i++) {
                            this->displayType[i] = DisplayType::Hide;
                        }

                        uint8_t diceValue = this->getDice_OfaKind(2, 0);
                        for (uint8_t i = 0; i < 5; i++) {
                            if (this->dice[i] == diceValue) {
                                this->displayType[i] = DisplayType::Display;
                            }
                        }

                        diceValue = this->getDice_OfaKind(2, diceValue);
                        for (uint8_t i = 0; i < 5; i++) {
                            if (this->dice[i] == diceValue) {
                                this->displayType[i] = DisplayType::Display;
                            }
                        }

                    }
                    break;

                case HandType::Pair:
                    {
                        uint8_t diceValue = this->getDice_OfaKind(2, 0);
                        for (uint8_t i = 0; i < 5; i++) {
                            if (this->dice[i] != diceValue) {
                                this->displayType[i] = DisplayType::Hide;
                            }
                        }

                    }
                    break;

                case HandType::High_Roll:
                    {

                        for (uint8_t i = 0; i < 5; i++) {
                            this->displayType[i] = DisplayType::Hide;
                        }

                        uint8_t i = this->getFirstDice(1);
                        this->displayType[i] = DisplayType::Display;

                        i = this->getFirstDice(6);
                        this->displayType[i] = DisplayType::Display;

                    }
                    break;

            }
        
        }

        void addSkullToDeck(SkullType id) {

            if (this->deckCount < MAX_DECK) {
                this->deck[this->deckCount++].setSkullType(id);
            }

        }


        uint8_t countSkull(SkullType id) {

            uint8_t n = 0;

            for (uint8_t i = 0; i < this->deckCount; i++) {

                if (this->deck[i].getSkullType() == id) {
                    n++;
                }

            }

            return n;

        }

        void updateDeckEntry_Bones(SkullType id, uint8_t bones) {

            uint8_t n = 0;

            for (uint8_t i = 0; i < this->deckCount; i++) {

                if (this->deck[i].getSkullType() == id) {

                    this->deck[i].setBones(bones);
                    
                }

            }

        }

        void updateDeckEntry_Multiplier(SkullType id, uint8_t multiplier) {

            uint8_t n = 0;

            for (uint8_t i = 0; i < this->deckCount; i++) {

                if (this->deck[i].getSkullType() == id) {

                    this->deck[i].setMultiplier(multiplier);
                    
                }

            }

        }

        void resetDice() {

            for (uint8_t i = 0; i < 5; i++) {
                this->dice[i] = 7;
                this->displayType[i] = DisplayType::Display;
            }

        }

        void rollAll() {
        
            for (uint8_t i = 0; i < 5; i++) {

                this->displayType[i] = DisplayType::RollAgain;

            }
        
        }

        void rerollDice(uint8_t i) {

            this->dice[i] = random(1, 7);

        }

        uint8_t diceSum() {

            uint8_t s = 0;
            for (uint8_t i = 0; i < 5; i++) s += this->dice[i];
            return s;

        }

        uint16_t evaluateHand() {

            #ifdef DEBUG_HAND
                DEBUG_PRINTLN("\n--------------------------");
            #endif

            this->clearDeck();


            // How many of each dice do we have?

            uint8_t counts[7] = {0, 0, 0, 0, 0, 0, 0};
            for (uint8_t i = 0; i < 5; i++) counts[this->dice[i]]++;

            uint8_t maxCount = 0;
            for (uint8_t v = 1; v <= 6; v++) if (counts[v] > maxCount) maxCount = counts[v];

            uint8_t pairCount = 0, threeCount = 0;
            for (uint8_t v = 1; v <= 6; v++) {
                if (counts[v] == 2) pairCount++;
                if (counts[v] == 3) threeCount++;
            }

            bool straightLow  = counts[1] && counts[2] && counts[3] && counts[4] && counts[5];
            bool straightHigh = counts[2] && counts[3] && counts[4] && counts[5] && counts[6];
            bool isStraight = straightLow || straightHigh;

            bool allEven = true, allOdd = true;

            for (uint8_t i = 0; i < 5; i++) {
                
                if (this->dice[i] % 2 == 0) {
                    allOdd = false; 
                }
                else {
                    allEven = false;
                }

            }

            uint8_t sixCount = counts[6];
            uint8_t aceCount = counts[1];

            uint16_t handBones = 0;
            uint16_t skullBones = 0;
            uint16_t upgradeBones = 0;
            uint8_t handMultiplier = 0;
            uint8_t skullMultiplier = 0;
            uint8_t upgradeMultiplier = 0;

            HandType handType = HandType::None; 

            if (maxCount == 5) {

                handBones = 30; 
                handMultiplier = 14; 
                handType = HandType::Five_of_a_Kind;

                if (this->upgradeHand == HandType::Five_of_a_Kind) {
                
                    upgradeBones = 5;
                    upgradeMultiplier = 1;
                    
                }

                #ifdef DEBUG_HAND
                    DEBUG_PRINTLN("Five of a kind: B 30, M 14");
                #endif

            } 
            else if (isStraight) {

                handBones = 30; 
                handMultiplier = 11; 
                handType = HandType::Straight;

                if (this->upgradeHand == HandType::Straight) {
                
                    upgradeBones = 5;
                    upgradeMultiplier = 1;
                    
                }
                
                #ifdef DEBUG_HAND
                    DEBUG_PRINTLN("Straight: B 30, M 11");
                #endif

            } 
            else if (maxCount == 4) {

                handBones = 25; 
                handMultiplier = 8; 
                handType = HandType::Four_of_a_Kind;

                if (this->upgradeHand == HandType::Four_of_a_Kind) {
                
                    upgradeBones = 5;
                    upgradeMultiplier = 1;
                    
                }
                
                #ifdef DEBUG_HAND
                    DEBUG_PRINTLN("Four of a Kind: B 25, M 8");
                #endif

            } 
            else if (maxCount == 3 && pairCount == 1) {

                handBones = 20; 
                handMultiplier = 6; 
                handType = HandType::Full_House;
                
                if (this->upgradeHand == HandType::Full_House) {
                
                    upgradeBones = 5;
                    upgradeMultiplier = 1;
                    
                }

                #ifdef DEBUG_HAND
                    DEBUG_PRINTLN("Full House: B 20, M 6");
                #endif
            } 
            else if (threeCount == 1) {

                handBones = 15; 
                handMultiplier = 4; 
                handType = HandType::Three_of_a_Kind;
                
                if (this->upgradeHand == HandType::Three_of_a_Kind) {
                
                    upgradeBones = 5;
                    upgradeMultiplier = 1;
                    
                }

                #ifdef DEBUG_HAND
                    DEBUG_PRINTLN("Three of a Kind: B 15, M 4");
                #endif

            } 
            else if (pairCount == 2) {

                handBones = 10; 
                handMultiplier = 3; 
                handType = HandType::Two_Pair;
                
                if (this->upgradeHand == HandType::Two_Pair) {
                
                    upgradeBones = 5;
                    upgradeMultiplier = 1;
                    
                }

                #ifdef DEBUG_HAND
                    DEBUG_PRINTLN("Two Pair: B 10, M 3");
                #endif

            } 
            else if (pairCount == 1) {

                handBones = 5; 
                handMultiplier = 2; 
                handType = HandType::Pair;
                
                if (this->upgradeHand == HandType::Pair) {
                
                    upgradeBones = 5;
                    upgradeMultiplier = 1;
                    
                }

                #ifdef DEBUG_HAND
                    DEBUG_PRINTLN("Pair: B 5, M 2");
                #endif

            } 
            else if (sixCount == 1 && aceCount == 1) {

                handBones = 5; 
                handMultiplier = 2; 
                handType = HandType::High_Roll;
                
                if (this->upgradeHand == HandType::High_Roll) {
                
                    upgradeBones = 5;
                    upgradeMultiplier = 1;
                    
                }

                #ifdef DEBUG_HAND
                    DEBUG_PRINTLN("High: B 5, M 2");
                #endif

            } 
            else {

                handBones = 0; 
                handMultiplier = 1; 
                handType = HandType::None;
                
                #ifdef DEBUG_HAND
                    DEBUG_PRINTLN("Nothing: B 0, M 0");
                #endif

            }

            if (handType == HandType::High_Roll && this->countSkull(SkullType::High_Roll_Save) > 0) {

                handBones = 15;

                #ifdef DEBUG_HAND
                    DEBUG_PRINTLN("Nothing + Save: B 15, M 0");
                #endif

            }



            // Skulls ---------------------------------------------------------------------------------------

            // skull-driven mult bonuses

            if (this->countSkull(SkullType::Pair_Multiplier) > 0 && (handType == HandType::Pair || handType == HandType::Two_Pair)) {

                skullMultiplier += this->countSkull(SkullType::Pair_Multiplier);
                updateDeckEntry_Multiplier(SkullType::Pair_Multiplier, 1);

                #ifdef DEBUG_HAND
                    DEBUG_PRINTLN("Sk Pair_Multiplier: B 0, M 1");
                #endif

            }

            if (this->countSkull(SkullType::Big_Multiplier) > 0 && (handType == HandType::Straight || handType == HandType::Full_House)) {

                skullMultiplier += this->countSkull(SkullType::Big_Multiplier);
                updateDeckEntry_Multiplier(SkullType::Big_Multiplier, 1);

                #ifdef DEBUG_HAND
                    DEBUG_PRINTLN("Sk Big_Multiplier: B 0, M 1");
                #endif

            }

            if (this->countSkull(SkullType::Kind_3or4_Multiplier) > 0 && (handType == HandType::Three_of_a_Kind || handType == HandType::Four_of_a_Kind)) {

                skullMultiplier += this->countSkull(SkullType::Kind_3or4_Multiplier);
                updateDeckEntry_Multiplier(SkullType::Kind_3or4_Multiplier, 1);

                #ifdef DEBUG_HAND
                    DEBUG_PRINTLN("Sk Kind_3or4_Multiplier: B 0, M 1");
                #endif

            }

            if (this->countSkull(SkullType::Kind_345_Multiplier) > 0 && (handType == HandType::Three_of_a_Kind || handType == HandType::Four_of_a_Kind || handType == HandType::Five_of_a_Kind)) {

                skullMultiplier += this->countSkull(SkullType::Kind_345_Multiplier);
                updateDeckEntry_Multiplier(SkullType::Kind_345_Multiplier, 3);

                #ifdef DEBUG_HAND
                    DEBUG_PRINTLN("Sk Kind_345_Multiplier: B 0, M 3");
                #endif

            }

            if (this->countSkull(SkullType::Even_Mulitplier) > 0 && allEven) {

                skullMultiplier += this->countSkull(SkullType::Even_Mulitplier);
                updateDeckEntry_Multiplier(SkullType::Even_Mulitplier, 1);

                #ifdef DEBUG_HAND
                    DEBUG_PRINTLN("Sk Even_Multiplier: B 0, M 1");
                #endif

            }

            if (this->countSkull(SkullType::Odd_Mulitplier) > 0 && allOdd)  {

                skullMultiplier += this->countSkull(SkullType::Odd_Mulitplier);
                updateDeckEntry_Multiplier(SkullType::Odd_Mulitplier, 1);

                #ifdef DEBUG_HAND
                    DEBUG_PRINTLN("Sk Odd_Mulitplier: B 0, M 1");
                #endif

            }

            // skull-driven chip bonuses

            if (this->countSkull(SkullType::Flat_Bones) > 0) {

                skullBones += this->countSkull(SkullType::Flat_Bones) * 10;
                updateDeckEntry_Bones(SkullType::Flat_Bones, 10);

                #ifdef DEBUG_HAND
                    DEBUG_PRINT("Sk Flat_Bones: B ");
                    DEBUG_PRINT(this->countSkull(SkullType::Flat_Bones) * 10);
                    DEBUG_PRINTLN(", M 0");
                #endif

            }

            if (this->countSkull(SkullType::Six_Bonus) > 0) {

                skullBones += this->countSkull(SkullType::Six_Bonus) * 3 * sixCount;
                updateDeckEntry_Bones(SkullType::Six_Bonus, 3 * sixCount);

                #ifdef DEBUG_HAND
                    DEBUG_PRINT("Sk Six_Bonus: B ");
                    DEBUG_PRINT(this->countSkull(SkullType::Six_Bonus) * 3 * sixCount);
                    DEBUG_PRINTLN(", M 0");
                #endif

            }

            if (this->countSkull(SkullType::Ace_Bonus) > 0) {

                skullBones += this->countSkull(SkullType::Ace_Bonus) * 3 * aceCount;
                updateDeckEntry_Bones(SkullType::Ace_Bonus, 3 * aceCount);

                #ifdef DEBUG_HAND
                    DEBUG_PRINT("Sk Ace_Bonus: B ");
                    DEBUG_PRINT(this->countSkull(SkullType::Six_Bonus) * 3 * sixCount);
                    DEBUG_PRINTLN(", M 0");
                #endif

            }

            if (this->countSkull(SkullType::No_Reroll_Bonus) > 0 && !this->rerollUsedThisHand) {

                skullBones += this->countSkull(SkullType::No_Reroll_Bonus) * 15;
                updateDeckEntry_Bones(SkullType::No_Reroll_Bonus, 15);

                #ifdef DEBUG_HAND
                    DEBUG_PRINT("Sk No_Reroll_Bonus: B ");
                    DEBUG_PRINT(this->countSkull(SkullType::No_Reroll_Bonus) * 15);
                    DEBUG_PRINTLN(", M 0");
                #endif

            }

            if (this->countSkull(SkullType::Mountainous) >  0 && this->dice[0] + 2 == this->dice[1] + 1 && this->dice[1] + 1 == this->dice[2] && this->dice[2] == this->dice[3] + 1 && this->dice[3] + 1 == this->dice[4] + 2) {

                skullMultiplier += this->countSkull(SkullType::Mountainous) * 3;
                updateDeckEntry_Multiplier(SkullType::Mountainous, 3);

                #ifdef DEBUG_HAND
                    DEBUG_PRINT("Sk Mountainous: B 0, M ");
                    DEBUG_PRINT(this->countSkull(SkullType::Mountainous) * 3);
                    DEBUG_PRINTLN("");
                #endif
                            
            }

            if (this->countSkull(SkullType::TwiceAsHigh) >  0 && sixCount == 2 && aceCount == 2) {

                skullMultiplier += this->countSkull(SkullType::TwiceAsHigh) * 3;
                updateDeckEntry_Multiplier(SkullType::TwiceAsHigh, 3);

                #ifdef DEBUG_HAND
                    DEBUG_PRINT("Sk TwiceAsHigh: B 0, M ");
                    DEBUG_PRINT(this->countSkull(SkullType::TwiceAsHigh) * 3);
                    DEBUG_PRINTLN("");
                #endif

            }

            if (this->countSkull(SkullType::StraightUp) >  0 && this->dice[0] == this->dice[1] - 1 && this->dice[1] == this->dice[2] - 1 && this->dice[2] == this->dice[3] - 1 && this->dice[3] == this->dice[4] - 1) {

                skullMultiplier += this->countSkull(SkullType::StraightUp) * 3;
                updateDeckEntry_Multiplier(SkullType::StraightUp, 3);

                #ifdef DEBUG_HAND
                    DEBUG_PRINT("Sk StraightUp: B 0, M ");
                    DEBUG_PRINT(this->countSkull(SkullType::StraightUp) * 3);
                    DEBUG_PRINTLN("");
                #endif
                            
            }            

            if (this->countSkull(SkullType::Sixes_45) >  0 && sixCount >= 4) {

                skullMultiplier += this->countSkull(SkullType::Sixes_45) * 3;
                updateDeckEntry_Multiplier(SkullType::Sixes_45, 3);

                #ifdef DEBUG_HAND
                    DEBUG_PRINT("Sk Sixes_45: B 0, M ");
                    DEBUG_PRINT(this->countSkull(SkullType::Sixes_45) * 3);
                    DEBUG_PRINTLN("");
                #endif

            }

            if (this->countSkull(SkullType::Them_Bones) > 0) {

                skullBones += this->countSkull(SkullType::Them_Bones) * 25;
                updateDeckEntry_Bones(SkullType::Them_Bones, 25);

                #ifdef DEBUG_HAND
                    DEBUG_PRINT("Sk Them_Bones: B ");
                    DEBUG_PRINT(this->countSkull(SkullType::Them_Bones) * 25);
                    DEBUG_PRINTLN(", M 0");
                #endif

            }

            this->lastHandScore.baseBones = this->diceSum();
            this->lastHandScore.handBones = handBones;
            this->lastHandScore.handMultiplier = handMultiplier;
            this->lastHandScore.skullBones = skullBones;
            this->lastHandScore.skullMultiplier = skullMultiplier;
            this->lastHandScore.upgradeBones = upgradeBones;
            this->lastHandScore.upgradeMultiplier = upgradeMultiplier;
            this->lastHandScore.totalBones = this->lastHandScore.baseBones + this->lastHandScore.handBones + this->lastHandScore.skullBones + this->lastHandScore.upgradeBones;
            this->lastHandScore.totalMultiplier = this->lastHandScore.handMultiplier + this->lastHandScore.skullMultiplier + this->lastHandScore.upgradeMultiplier;
            this->lastHandScore.handType = handType;

            #ifdef DEBUG_HAND
                DEBUG_PRINT("Base Bones ");
                DEBUG_PRINTLN(this->diceSum());
                DEBUG_PRINT("Hand Bones / Multiplier ");
                DEBUG_PRINT(handBones);
                DEBUG_PRINT(", ");
                DEBUG_PRINTLN(handMultiplier);
                DEBUG_PRINT("Skull Bones / Multiplier ");
                DEBUG_PRINT(skullBones);
                DEBUG_PRINT(", ");
                DEBUG_PRINTLN(skullMultiplier);
                DEBUG_PRINT("Upgrade Bones / Multiplier ");
                DEBUG_PRINT(upgradeBones);
                DEBUG_PRINT(", ");
                DEBUG_PRINTLN(upgradeMultiplier);
                DEBUG_PRINT("Total Bones / Multiplier ");
                DEBUG_PRINT(this->lastHandScore.totalBones);
                DEBUG_PRINT(", ");
                DEBUG_PRINTLN(this->lastHandScore.totalMultiplier);
            #endif

            uint16_t score = this->lastHandScore.totalBones * this->lastHandScore.totalMultiplier;
            if (this->firstHandOfLevel && this->countSkull(SkullType::Double_First) > 0) score *= 2;
            this->lastHandScore.score = score;

            return score;

        }


        uint8_t getUpgradeHand(uint8_t indexToReturn) {

            uint8_t count = 0;

            for (int8_t i = static_cast<uint8_t>(HandType::Count) - 2; i >= 0; i--) {
            
                if (upgradeHandsToShow[i] == true) {

                    if (count == indexToReturn) {
                    
                        return i;

                    }
                
                    count++;

                }

            }
        
        return 0;
        }

        void selectUpgradeHands(uint8_t count) {
        
            for (uint8_t i = 0; i < 8; i++) {
                this->upgradeHandsToShow[i] = false;           
            }

            for (uint8_t i = 0; i < count; i++) {
            
                do {

                    uint8_t idx = random(0, static_cast<uint8_t>(HandType::Count) - 2);

                    if (this->upgradeHandsToShow[idx] == false) {
                        this->upgradeHandsToShow[idx] = true;
                        break;
                    }

                } while (true);
                
            }

        }

        HandType getUpgradeHandType(uint8_t indexToReturn) {

            uint8_t count = 0;
            for (uint8_t i = 0; i < static_cast<uint8_t>(HandType::Count) - 2; i++) {

                if (this->upgradeHandsToShow[i] == true) {

                    if (count == indexToReturn) {
                        return static_cast<HandType>(8 - i);
                    }

                    count++;

                }

            }

        return (HandType)0;
        }

};