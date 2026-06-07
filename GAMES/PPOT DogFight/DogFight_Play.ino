#include "src/utils/Arduboy2Ext.h"
#include "src/utils/Constants.h"

uint8_t lowestHP = 255;
uint8_t lowestHPCount = 0;


void initHeart() {

    EnemyPlane& h = enemies[Constants::Heart_Idx];
    // h.setX(FP8((int8_t)random(-50, 50)));
    // h.setZ(FP8((int8_t)random(-50, 50)));
    h.setX(FP8((int8_t)arduboy.randomLFSR(0, 100) - 50));
    h.setZ(FP8((int8_t)arduboy.randomLFSR(0, 100) - 50));

    h.setY(FP8(0));
    h.setYaw(FP16(0));
    h.setActive(true);
    h.setState(AIState::Inbound);
    h.setHP(1);

}


void handleInput(Player &player) {

    player.setPressed(arduboy.pressedButtons());
    player.setJustPressed(arduboy.justPressedButtons());

    if (player.fire()) fireWeapon(player);

}


uint8_t fireWeapon(Player &cam) {

    flashFrames = 4;

    static const uint8_t killRadius[6] PROGMEM = {3, 5, 8, 9, 11, 13};

    for (uint8_t i = 0; i < Constants::Entity_Count; i++) {

        EnemyPlane &entity = enemies[i];

        if (!entity.isActive())                     continue;
        if (entity.getState() == AIState::Explode)  continue;
        if (entity.getState() == AIState::Dead)     continue;

        FP8 dx = entity.getX() - player.getX();
        FP8 dz = entity.getZ() - player.getZ();
        Vec3        c  = worldToCameraFast(dx, dz, player.getCosY(), player.getSinY());
        ScreenPoint sp = project3d(c, entity.getY(), player.getY(), player.getPitch());

        if (!sp.valid) continue;

        uint8_t sz  = constrain(sp.size, 1, 6);
        uint8_t idx = sz - 1;

        uint8_t kr = pgm_read_byte(&killRadius[idx]);
        if (abs(sp.x - Constants::Screen_Half_W) <= kr && abs(sp.y - (Constants::Screen_Half_H - 4)) <= kr) {

            if (i == Constants::Heart_Idx) {

                uint8_t hp = player.getHP();
                if (hp < 15) player.setHP(min((uint8_t)(hp + 3), (uint8_t)15));
                entity.setState(AIState::Explode);
                entity.setTimer(Constants::Heart_Respawn_Frames + (player.getScore() * 2));
                flashFrames = 1;

            }
            else {

                entity.setHP(entity.getHP() - 1);

                if (entity.getHP() == 0) {

                    entity.setState(AIState::Explode);
                    entity.setTimer(Constants::Enemy_Respawn_Frames);
                    flashFrames = 1;
                    player.setScore(player.getScore() + 1);
                    if (speedFactor < Constants::SpeedFactor_Max) speedFactor += Constants::SpeedFactor_Inc;

                }

                if (entity.getHP() < lowestHP) {
                    lowestHP = entity.getHP();
                    lowestHPCount = 32;
                }

            }

            break;

        }

    }

    return lowestHP;

}



void render(Player &player) {

    // Compute player camera trig ONCE for this entire frame
    
    cameraCosSin(player);

    drawHorizon(arduboy, player, hs, gameState);
    drawClouds();
    drawEnemies();   // heart pickup is enemies[Heart_Idx], drawn inside drawEnemies


   // Build legacy Enemy array for radar (Entity_Count includes the heart pickup)

    RadarEnemy radarEnemies[Constants::Entity_Count];

    for (uint8_t i = 0; i < Constants::Entity_Count; i++) {

        radarEnemies[i].setX(enemies[i].getX());
        radarEnemies[i].setZ(enemies[i].getZ());
        radarEnemies[i].setActive(enemies[i].isActive());

    }

    if (flashFrames > 0) {

        uint8_t frame = (flashFrames / 2) % 2;
        Sprites::drawExternalMask(Constants::Screen_Half_W - 13, Constants::Screen_Half_H - 16, Images::Player_Fire_02, Images::Player_Fire_02_Mask, frame, frame);
        flashFrames--;

    }


    // BiPlane image has radar frame baked into image ..

    Sprites::drawExternalMask(0, 0, Images::Biplane, Images::Biplane_Mask, 0, 0);




    // Pass precomputed trig — no extra fpCos/fpSin inside drawRadar

    drawRadar(arduboy, player, radarEnemies, Constants::Entity_Count);

   
    if (gameState == GameState::Playing) {

        uint16_t score   = player.getScore();
        uint8_t hundreds = score / 100;
        uint8_t tens     = (score / 10) % 10;
        uint8_t units    = score % 10;

        Sprites::drawOverwrite(0, 57, Images::ScoreMask, 0);

        Sprites::drawOverwrite(0, 59, Images::Score_Nums, hundreds);
        Sprites::drawOverwrite(4, 59, Images::Score_Nums, tens);
        Sprites::drawOverwrite(8, 59, Images::Score_Nums, units);


        // Render pitch on dashboard ..

        int8_t pitInt = ((int8_t)(player.getPitch() / 7.5) + 6);
        if (pitInt < 0)     pitInt = 0;
        if (pitInt > 11)    pitInt = 11;
        Sprites::drawOverwrite(49, 45, Images::Pitch, pitInt);

        // Render Yaw on dashboard ..

        int8_t yawInt = ((int8_t)(player.getYawRate()) + 7);
        if (yawInt < 0)     yawInt = 0;
        if (yawInt > 14)    yawInt = 14;
        Sprites::drawOverwrite(65, 45, Images::Yaw, yawInt);

    }
    else {

        Sprites::drawOverwrite(50, 46, Images::GameOver_Eyes, 0);

    }


    // Render player health ..

    {
        uint8_t hp = player.getHP();

        Sprites::drawOverwrite(104, 30, Images::Hearts, min(hp, 5));
        Sprites::drawOverwrite(112, 30, Images::Hearts, (uint8_t)constrain((int8_t)hp - 5,  0, 5));
        Sprites::drawOverwrite(120, 30, Images::Hearts, (uint8_t)constrain((int8_t)hp - 10, 0, 5));                
    
    }

    if (hitFlashFrames > 0) {

        arduboy.invert(true);
        hitFlashFrames--;

        if (hitFlashFrames == 0 && player.getHP() == 0) {

            gameState = GameState::GameOver_Init;

        }

    } 
    else {
        arduboy.invert(false);
    }

    if (behindYouFrames > 0) {
        Sprites::drawOverwrite(0, -6, Images::BehindYou, 0);
        behindYouFrames--;
    }

    if (lowestHP != 255 && lowestHPCount > 0) {

        lowestHPCount--;

        Sprites::drawExternalMask(Constants::Screen_Half_W - 13, 0, Images::EnemyHeart, Images::EnemyHeart_Mask, lowestHP, lowestHP);

        if (lowestHP == 0) {

            uint16_t score   = player.getScore();            
            uint8_t hundreds = score / 100;
            uint8_t tens     = (score / 10) % 10;
            uint8_t units    = score % 10;

            Sprites::drawOverwrite(Constants::Screen_Half_W - 1, 1, Images::EnemyScore, hundreds);
            Sprites::drawOverwrite(Constants::Screen_Half_W + 3, 1, Images::EnemyScore, tens);
            Sprites::drawOverwrite(Constants::Screen_Half_W + 7, 1, Images::EnemyScore, units);

        }

        if (lowestHPCount == 0) {

            lowestHP = 255;

        }

    }

}


// Projects a camera-space point accounting for altitude delta and pitch.

ScreenPoint project3d(Vec3 cam, FP8 targetY, FP8 playerY, FP8 pitchDeg) {

    if (cam.z < Constants::Closest_Distance) return { 0, Constants::Screen_Half_H, 0, false };

    FP8 invZ = getInvZ(cam.z);

    int16_t sx = Constants::Screen_Half_W + ((cam.x * invZ).getInternal() >> 2);   

    FP8     altDiff  = targetY - playerY;
    int8_t altPx = (int8_t)((altDiff * invZ).getInternal() >> 2);
    int8_t pitchPx  = (int8_t)(pitchDeg * Constants::Pitch_PX_Per_Deg);

    int8_t sy       = (int8_t)(Constants::Screen_Half_H - altPx + pitchPx);
    uint8_t distInt = (uint8_t)constrain((int16_t)cam.z, 1, 127);
    uint8_t sz      = (uint8_t)constrain(40 / distInt, 1, 6);

    if (sx < -8 || sx > Constants::Screen_W + 8) return { 0, Constants::Screen_Half_H, 0, false };
    return { (int8_t)sx, sy, sz, true };
}



// ── movePlayer ───────────────────────────────────────────────────────
void movePlayer(Player& player) {


    // ── 1. Airspeed: boost toward Constants::Boost_Speed_Max, decay toward Constants::Cruise_Speed ──

    if (player.boost()) {
        player.setVelFwd(min(player.getVelFwd() + Constants::Boost_Acc, Constants::Boost_Speed_Max));
    } 
    else {
        FP8 v = player.getVelFwd() * Constants::Boost_Drag;
        if (v < Constants::Cruise_Speed) v = Constants::Cruise_Speed;
        player.setVelFwd(v);
    }


    // ── 2. Pitch rate with inertia ────────────────────────────────────────

    FP8 pitchRate = player.getPitchRate();

    if (player.pitchUp()) {
        pitchRate = min(pitchRate + Constants::Pitch_Rate_Acc,  Constants::Pitch_Rate_Max);
    }
    if (player.pitchDown()) {
        pitchRate = max(pitchRate - Constants::Pitch_Rate_Acc, -Constants::Pitch_Rate_Max);
    }

    pitchRate = pitchRate * Constants::Pitch_Rate_Drag;
    if (pitchRate > FP8(-0.05) && pitchRate < FP8(0.05)) {
        pitchRate = FP8(0);
    }

    player.setPitchRate(pitchRate);

    FP8 pitch = player.getPitch() + pitchRate;
    if (pitch >  Constants::Bank_Deg_Max) pitch =  Constants::Bank_Deg_Max;
    if (pitch < -Constants::Bank_Deg_Max) pitch = -Constants::Bank_Deg_Max;

    if (!player.pitchUp() && !player.pitchDown()) {
        pitch = pitch * Constants::Pitch_Gravity;
    }
    player.setPitch(pitch);


    // ── 3. Decompose airspeed into horizontal + vertical ──────────────────

    int16_t  pitchInt = (int16_t)pitch;
    uint16_t pitchU   = (uint16_t)((pitchInt % 360 + 360) % 360);
    FP8 cosP = fpCos(pitchU);
    FP8 sinP = fpSin(pitchU);

    FP8 horizSpeed = player.getVelFwd() * cosP;
    FP8 thrustY    = player.getVelFwd() * sinP;


    // ── 4. Horizontal movement ────────────────────────────────────────────

    player.setX(player.getX() + fpSin(player.getYaw().getInteger()) * horizSpeed);
    player.setZ(player.getZ() + fpCos(player.getYaw().getInteger()) * horizSpeed);


    // ── 5. Vertical movement ──────────────────────────────────────────────

    FP8 velY = thrustY - Constants::Pitch_Sink_Rate;
    FP8 y    = player.getY() + velY;

    if (y < Constants::Altitude_Min) { y = Constants::Altitude_Min; velY = FP8(0); }
    if (y > Constants::Altitude_Max) { y = Constants::Altitude_Max; velY = FP8(0); }
    player.setY(y);
    player.setVelY(velY);


    // ── 6. Yaw with inertia ───────────────────────────────────────────────

    FP8 yawRate = player.getYawRate();

    if (player.yawLeft()) {
        yawRate = max(yawRate - Constants::Yaw_Rate_Acc, -Constants::Yaw_Rate_Max);
    }
    if (player.yawRight()) {
        yawRate = min(yawRate + Constants::Yaw_Rate_Acc,  Constants::Yaw_Rate_Max);
    }

    yawRate = yawRate * Constants::Yaw_Rate_Drag;
    if (yawRate > FP8(-0.05) && yawRate < FP8(0.05))
        yawRate = FP8(0);
    player.setYawRate(yawRate);

    int16_t newYaw = (int16_t)player.getYaw() + (int16_t)yawRate;
    player.setYaw((uint16_t)((newYaw % 360 + 360) % 360));

}


void gameUpdate() {

    handleInput(player);
    movePlayer(player);
    updateHorizonState(hs, player);

    // Update all enemy AIs — updateEnemyAI handles dead/respawn internally
    for (uint8_t i = 0; i < Constants::Enemy_Count; i++) {

        uint8_t shotResult = updateEnemyAI(arduboy, enemies[i], player.getX(), player.getZ(), player.getY(), player.getYaw(), i, speedFactor);

        if (shotResult > 0 && player.getHP() > 0) {
            player.decHP(2);
            hitFlashFrames = 3;
            if (shotResult == 2) behindYouFrames = 30;
        }

    }

    // Heart pickup: animate explosion and respawn
    {
        EnemyPlane& heart = enemies[Constants::Heart_Idx];

        if (heart.getState() == AIState::Explode) {

            heart.incExplosionFrame();

            if (heart.getExplosionFrame() >= 8) {
                heart.setState(AIState::Dead);
                heart.setActive(false);
                heart.setExplosionFrame(0);
            }

        } 
        else if (!heart.isActive()) {

            if (heart.getTimer() > 0) {
                heart.setTimer(heart.getTimer() - 1);
            }
            else {
                initHeart();
            }

        }

    }

    render(player);

}