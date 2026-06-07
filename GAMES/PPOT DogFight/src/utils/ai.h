
// Movement convention: yaw=0 → +Z, yaw=90 → +X, yaw=180 → -Z, yaw=270 → -X

#pragma once

#include "Arduboy2Ext.h"
#include "Constants.h"
#include "Maths.h"


static void moveEnemy(EnemyPlane& enemy, FP8 speedFactor) {
    enemy.setX(enemy.getX() + fpSin(enemy.getYaw().getInteger()) * enemy.getSpeed() * speedFactor);
    enemy.setZ(enemy.getZ() + fpCos(enemy.getYaw().getInteger()) * enemy.getSpeed() * speedFactor);
}

// ── steerToward ────────────────────────────────────────────────────────────
static void steerToward(EnemyPlane& enemy, uint16_t target, int16_t rate) {

    int16_t diff = angleDiff(enemy.getYaw().getInteger(), target);
    if (diff >  rate) diff =  rate;
    if (diff < -rate) diff = -rate;

    FP16 newNav = enemy.getYaw() + diff;
    if (newNav < -360) newNav = newNav + 360;
    if (newNav > 360)  newNav = newNav - 360;
    enemy.setYaw(newNav);

}

// ─────────────────────────────────────────────────────────────────────────────
// Place enemy at Enemy_Spawn_Dist ahead of the player (within ±60° of their
// heading). Uses timer+idx bits for variety so each spawn/enemy differs.
static void spawnEnemy(Arduboy2Ext &arduboy, EnemyPlane& enemy, FP8 px, FP8 pz, FP8 py, FP16 playerYaw, uint8_t idx) {

    const int8_t offsets[3] = {-20, 0, 20};

    #ifndef DEBUG

        // int16_t  variance   = random(360) - 180;
        int16_t  variance   = (arduboy.randomLFSR(0, 180) * 2) - 180;
        uint16_t spawnAngle = (uint16_t)(((int16_t)playerYaw.getInteger() + variance + 360) % 360);

        enemy.setX(px + FP8((int8_t)((int16_t)(fpSin(spawnAngle).getInternal() * Constants::Enemy_Spawn_Dist) >> 8)));
        enemy.setZ(pz + FP8((int8_t)((int16_t)(fpCos(spawnAngle).getInternal() * Constants::Enemy_Spawn_Dist) >> 8)));
        enemy.setY(py);

        uint16_t playerBearing = bearing(enemy.getX(), enemy.getZ(), px, pz);
        enemy.setPassOffsetDeg(offsets[idx % 3]);
        enemy.setArcDir((idx % 2 == 0) ? 1 : -1);
        enemy.setYaw(FP16(playerBearing));

    #else

        int16_t distance = Constants::Enemy_Spawn_Dist;

        FP8 offsetX = FP8((int8_t)((int16_t)(fpSin(playerYaw.getInteger()).getInternal() * distance) >> 8));
        FP8 offsetZ = FP8((int8_t)((int16_t)(fpCos(playerYaw.getInteger()).getInternal() * distance) >> 8));

        enemy.setX(px + offsetX);
        enemy.setZ(pz + offsetZ);
        enemy.setY(10); 

        uint16_t playerBearing = bearing(enemy.getX(), enemy.getZ(), px, pz);
        enemy.setYaw(FP16(playerBearing));

    #endif

    enemy.setSpeed(Constants::Enemy_Speeds[idx % 3]);
    enemy.setState(AIState::Inbound);
    enemy.setTimer(0);
    enemy.setActive(true);
    enemy.setD2Prev(30000);

}

// ─────────────────────────────────────────────────────────────────────────────

inline void initEnemies(Arduboy2Ext &arduboy, EnemyPlane* enemy, uint8_t count) {

    for (uint8_t i = 0; i < count; i++) {

        enemy[i].setHP(5);
        enemy[i].setTimer(i * 30);
        spawnEnemy(arduboy, enemy[i], FP8(0), FP8(0), FP8(0), 0, i);

    }

}

// ─────────────────────────────────────────────────────────────────────────────

// Returns 0 = no hit, 1 = hit from front, 2 = hit from behind
uint8_t updateEnemyAI(Arduboy2Ext &arduboy, EnemyPlane &enemy, FP8 px, FP8 pz, FP8 py, FP16 playerYaw, uint8_t idx, FP8 speedFactor = FP8(1.0)) {

    // Dead: count down then respawn ahead of the player

    if (!enemy.isActive()) {
        if (enemy.getTimer() > 0) { enemy.setTimer(enemy.getTimer() - 1); return 0; }
        enemy.setHP(5);
        spawnEnemy(arduboy, enemy, px, pz, py, playerYaw, idx);
        return 0;
    }

    // Failsafe: wandered too far → respawn

    int16_t distance2 = distanceSquared(enemy.getX(), enemy.getZ(), px, pz);

    if (distance2 > Constants::Enemy_Spawn_Dist2) {

        #ifdef DEBUG
            Serial.print(F("[FAILSAFE d2="));   Serial.print(distance2);
            Serial.print(F(" state="));         Serial.println((uint8_t)enemy.getState());
        #endif

        spawnEnemy(arduboy, enemy, px, pz, py, playerYaw, idx);
        return 0;
    }


    if (enemy.getMuzzleFlashFrames() > 0) enemy.setMuzzleFlashFrames(enemy.getMuzzleFlashFrames() - 1);

    // Work out where the enemy is relatie to the player and adjust speed accordingly ..

    uint16_t bearingToPlayer = bearing(enemy.getX(), enemy.getZ(), px, pz);
    uint16_t bearingToEnemy = (bearingToPlayer + 180) % 360;
    int16_t  relAngleEtoP = angleDiff(enemy.getYaw().getInteger(), bearingToPlayer);
    int16_t relAnglePtoE = angleDiff(playerYaw.getInteger(), bearingToEnemy);

    bool isInFront = (relAnglePtoE >= -90 && relAnglePtoE <= 90);
    bool isHeadingTowards = (relAngleEtoP >= -45 && relAngleEtoP <= 45);

    #ifdef DEBUG

        if (isInFront && isHeadingTowards)      Serial.println("Enemy is in front of you, flying at your face.");
        if (isInFront && !isHeadingTowards)     Serial.println("Enemy is in front of you, but flying away (you are chasing them).");
        if (!isInFront && isHeadingTowards)     Serial.println("Enemy is behind you, closing in on your tail.");
        if (!isInFront && !isHeadingTowards)    Serial.println("Enemy is behind you and flying further away.");

    #endif

    // isInFront → slow (0); behind+closing → fast (2); behind+away → mid (1)
    enemy.setSpeed(Constants::Enemy_Speeds[isInFront ? 0 : (isHeadingTowards ? 2 : 1)]);


    switch (enemy.getState()) {

        // ── INBOUND: fly toward player with a slight offset bias ───────────────

        case AIState::Inbound: {
            
            enemy.incTimer();

            #ifdef DEBUG
                
                if (enemy.getTimer() % 15 == 1) {
                    Serial.print(F("[IN E t="));            Serial.print(enemy.getTimer());
                    Serial.print(F(" yaw="));               Serial.print(enemy.getYaw().getInteger());
                    Serial.print(F(" brg="));               Serial.print(bearingToPlayer);
                    Serial.print(F(" rel="));               Serial.print(relAngleEtoP);
                    Serial.print(F(" d2="));                Serial.print(distance2);
                    Serial.print(F(" : P yaw="));           Serial.println(playerYaw.getInteger());
                }

            #endif

            // Only steer when player is well within the forward cone.  A wide window
            // let the enemy track a side-on or behind player, preventing relAngle > 110°.

            if (relAngleEtoP > -30 && relAngleEtoP < 30 && enemy.getTimer() % 30 == 0) {

                int16_t  biased    = (int16_t)bearingToPlayer + enemy.getPassOffsetDeg();
                uint16_t targetYaw = (uint16_t)((biased + 360) % 360);
                steerToward(enemy, targetYaw, (int16_t)Constants::Enemy_Turn_Rate);

            }


            // Move the enemy forward ..

            moveEnemy(enemy, speedFactor);


            // Adjust the enemy altitude ..

            if      (enemy.getY() < py - FP8(2)) enemy.setY(enemy.getY() + FP8(0.05));
            else if (enemy.getY() > py + FP8(2)) enemy.setY(enemy.getY() - FP8(0.05));


            // Enemy has flown past the player (player in enemy's rear hemisphere) → turn back ..

            if ((relAngleEtoP > 90 || relAngleEtoP < -90) && enemy.getTimer() > 10) {

                #ifdef DEBUG

                    Serial.print(F("[IN E t="));            Serial.print(enemy.getTimer());
                    Serial.print(F(" yaw="));               Serial.print(enemy.getYaw().getInteger());
                    Serial.print(F(" brg="));               Serial.print(bearingToPlayer);
                    Serial.print(F(" rel="));               Serial.print(relAngleEtoP);
                    Serial.print(F(" d2="));                Serial.print(distance2);
                    Serial.print(F(" : P yaw="));           Serial.println(playerYaw.getInteger());
                    Serial.print(F("[> TURN rel="));        Serial.print(relAngleEtoP);
                    Serial.print(F(" arc="));               Serial.println(enemy.getArcDir());

                #endif

                enemy.setArcDir((relAngleEtoP > 0) ? 1 : -1);
                enemy.setState(AIState::Turning);
                enemy.setTimer(0);
                break;

            }

            // Check to see if the enemy and player are getting further apart (distance2 growing) ..
            // Guard: only applies when player is still in front (isInFront), otherwise the turn transition above fires instead.

            if (enemy.getTimer() % 30 == 0) {

                if (enemy.getTimer() > 30 && distance2 > enemy.getD2Prev() + 30 && relAngleEtoP > -60 && relAngleEtoP < 60) {
                    // Player pulling away — snap directly toward them and re-approach.

                    #ifdef DEBUG
                    
                        Serial.print(F("[REFACE t="));      Serial.print(enemy.getTimer());
                        Serial.print(F(" d2="));            Serial.print(distance2);
                        Serial.print(F(" prev="));          Serial.println(enemy.getD2Prev());
                    
                    #endif

                    uint16_t playerBearing = bearing(enemy.getX(), enemy.getZ(), px, pz);
                    enemy.setYaw(FP16(playerBearing));
                    enemy.setD2Prev(distance2);
                    enemy.setTimer(0);
                    break;
                    
                }

                enemy.setD2Prev(distance2);

            }

            break;
        }

        // ── TURNING: fixed-rate arc back toward player, then re-engage ─────────
        case AIState::Turning: {

            enemy.incTimer();

            // Slow down while turning so the arc is tighter and the enemy stays close
            enemy.setSpeed(Constants::Enemy_Speeds[0]);

            // Change the angle of the enemy using the Enemy_Turn_Rate and the arc direction preiously sey ..

            FP16 newYaw = enemy.getYaw() + (enemy.getArcDir() * Constants::Enemy_Turn_Rate);
            if (newYaw < -360) newYaw = newYaw + 360;
            if (newYaw > 360)  newYaw = newYaw - 360;
            enemy.setYaw(newYaw);


            // Move the enemy ..

            moveEnemy(enemy, speedFactor);


            // Work out the angle between the enemy and player to see if we are now facing the player ..

            bearingToPlayer = bearing(enemy.getX(), enemy.getZ(), px, pz);
            relAngleEtoP = angleDiff(enemy.getYaw().getInteger(), bearingToPlayer);


            #ifdef DEBUG

                if (enemy.getTimer() % 20 == 0) {
                    Serial.print(F("[TRN t="));     Serial.print(enemy.getTimer());
                    Serial.print(F(" yaw="));       Serial.print(enemy.getYaw().getInteger());
                    Serial.print(F(" brg="));       Serial.print(bearingToPlayer);
                    Serial.print(F(" rel="));       Serial.println(relAngleEtoP);
                }

            #endif

            if (enemy.getTimer() > 20 && relAngleEtoP > -25 && relAngleEtoP < 25) {

                #ifdef DEBUG

                    Serial.print(F("[>IN rel="));   Serial.println(relAngleEtoP);

                #endif

                enemy.setPassOffsetDeg(-enemy.getPassOffsetDeg());
                enemy.setD2Prev(distance2);   // seed the divergence check for this re-approach
                enemy.setState(AIState::Inbound);
                enemy.setTimer(0);

            }

            break;

        }

        case AIState::Explode:

            enemy.incExplosionFrame();

            if (enemy.getExplosionFrame() >= 8) {
                enemy.setState(AIState::Dead);
                enemy.setActive(false);
                enemy.setExplosionFrame(0);
            }
            break;

        case AIState::Dead:
            break;

        default:
            enemy.setState(AIState::Inbound);
            break;

    }

    if (enemy.getY() < FP8(-30)) enemy.setY(FP8(-30));
    if (enemy.getY() > FP8(120)) enemy.setY(FP8(120));

    // ── Shooting ──────────────────────────────────────────────────────────────
    // Only Inbound/Turning enemies can fire; skip Explode/Dead.
    if (enemy.getState() != AIState::Inbound && enemy.getState() != AIState::Turning)
        return 0;

    if (enemy.getShotCooldown() > 0) {
        enemy.setShotCooldown(enemy.getShotCooldown() - 1);
        return 0;
    }


    // Enemy must be roughly aligned with the player and within firing range.

    if (distance2 < Constants::Enemy_Shot_Range2 &&
        relAngleEtoP > -Constants::Enemy_Shot_Cone &&
        relAngleEtoP <  Constants::Enemy_Shot_Cone) {

        enemy.setShotCooldown(Constants::Enemy_Shot_Cooldown);
        enemy.setMuzzleFlashFrames(4);

        // if (random(Constants::Enemy_Shot_Hit_Chance) == 0) {
        if (arduboy.randomLFSR(0, Constants::Enemy_Shot_Hit_Chance) == 0) {
            enemy.setShotCooldown(Constants::Enemy_Shot_Cooldown_Ext);
            return isInFront ? 1 : 2;
        }

    }

    return 0;

}
