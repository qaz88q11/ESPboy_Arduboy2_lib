#pragma once

#include "src/utils/Arduboy2Ext.h"
#include "src/utils/Constants.h"


void drawRadar_Blip(Arduboy2Ext& a, FP8 camX, FP8 camZ) {

    int16_t cx = (int16_t)camX;
    int16_t cz = (int16_t)camZ;

    int8_t rx = (int8_t)((cx * (int16_t)Constants::Radar_Radius) / Constants::Radar_Range);
    int8_t ry = (int8_t)((cz * (int16_t)Constants::Radar_Radius) / Constants::Radar_Range);

    int8_t bx = (int8_t)Constants::Radar_Centre_X + rx;
    int8_t by = (int8_t)Constants::Radar_Centre_Y - ry;   // negate: +Z forward = up on radar


    // Clip to bounding box

    if (bx < (int8_t)Constants::Radar_Left || bx >= Constants::Screen_W - 1) return;
    if (by < (int8_t)Constants::Radar_Top || by >= Constants::Screen_W - 1) return;

    a.drawPixel(bx, by, WHITE);

}


// Draw enemies on the radar if they are active ..

void drawRadar(Arduboy2Ext& a,
               const Player& player,
               const RadarEnemy*  enemies,
               uint8_t count) {

    for (uint8_t i = 0; i < count; i++) {

        RadarEnemy enemy = enemies[i];

        if (!enemy.isActive()) continue;

        FP8 dx =  enemy.getX() - player.getX();
        FP8 dz =  enemy.getZ() - player.getZ();
        FP8 cx =  dx * player.getCosY() + dz * player.getSinY();
        FP8 cz = -dx * player.getSinY() + dz * player.getCosY();

        drawRadar_Blip(a, cx, cz);

    }

}


static void drawExplosion(int8_t sx, int8_t sy, uint8_t sz, uint8_t frame) {
    uint8_t off = (sz == 1) ? 8 : (sz <= 3) ? 12 : 16;
    const uint8_t* img = (sz == 1) ? Images::Explosion_Sml : (sz <= 3) ? Images::Explosion_Med : Images::Explosion_Lrg;
    Sprites::drawSelfMasked(sx - off, sy - off, img, frame);
}

// ── drawEnemySprite ───────────────────────────────────────────────────────
// sp            — projected screen position and size
// headingToward — true = enemy nose pointing toward player → use Fwd sprite
//                 false = enemy flying away               → use Bck sprite
//
// Heading direction from the call site (computed once per enemy per frame):
//   FP8 vx = fpSin(enemies[i].yaw);
//   FP8 vz = fpCos(enemies[i].yaw);
//   bool toward = (vx * (-player.getSinY()) + vz * player.getCosY()) < FP8(0);
void drawEnemySprite(Arduboy2Ext& a, EnemyPlane &enemy, const ScreenPoint& sp, bool headingToward) {

    int8_t  sx = sp.x, sy = sp.y;


    // Clamp sz (size) to table range 1..6, then convert to 0-based index

    uint8_t sz  = constrain(sp.size, 1, 6);
    uint8_t idx = sz - 1;

    const EnemySpriteEntry* entry = &Images::Enemy_Sprites[idx];
    const uint8_t* img  = (const uint8_t*)pgm_read_dword(headingToward ? &entry->fwdImg  : &entry->bckImg);
    const uint8_t* mask = (const uint8_t*)pgm_read_dword(&entry->mask);

    int8_t offX = (int8_t)pgm_read_byte(&entry->offX);
    int8_t offY = (int8_t)pgm_read_byte(&entry->offY);
    
    Sprites::drawExternalMask(sx - offX, sy - offY, img, mask, 0, 0);

    if (enemy.getMuzzleFlashFrames() > 0) {

        if (gameState == GameState::Playing) {

            // Diagonal ray segments travel outward each frame — bolt moves away from enemy

            int16_t step     = (int16_t)(5 - (int8_t)enemy.getMuzzleFlashFrames());
            int16_t len      = step * 8;
            int16_t prev_len = (step - 1) * 8;

            a.drawLine(sx + prev_len, sy - prev_len, sx + len, sy - len, WHITE);
            a.drawLine(sx - prev_len, sy - prev_len, sx - len, sy - len, WHITE);
            a.drawLine(sx + prev_len, sy + prev_len, sx + len, sy + len, WHITE);
            a.drawLine(sx - prev_len, sy + prev_len, sx - len, sy + len, WHITE);

        }

    }

    if (enemy.getState() == AIState::Explode) drawExplosion(sx, sy, sz, enemy.getExplosionFrame());

}


void drawClouds() {

    uint16_t yawOffset = ((uint16_t)player.getYaw() * 64u) / 90u;

    int16_t hy_left   = horizonYatX(0,   player.getPitch(), hs.tiltAngle, hs.dipOffset);
    int16_t hy_right  = horizonYatX(127, player.getPitch(), hs.tiltAngle, hs.dipOffset);
    int16_t slopeFrac = ((hy_right - hy_left) * 128) / 127;

    for (uint8_t i = 0; i < Constants::Cloud_Count; i++) {

        uint16_t base_x = 5u + (uint16_t)i * 25u;
        uint8_t sy = pgm_read_byte(&Constants::CloudY[i]);
        uint16_t wrappedX = (base_x - yawOffset) & 255;
        int16_t sx = (int16_t)wrappedX;

        if (sx > 128) sx -= 256;


        // Ensure we are within the visible window (including negative buffer) ..

        if (sx >= -17 && sx < 128) {
            int8_t checkX = (sx < 0) ? 0 : (sx > 127 ? 127 : (int8_t)sx);
            int8_t hY = (int8_t)(hy_left + ((slopeFrac * checkX) >> 7));

            // If the top of the cloud (sy) is below the horizon, it's fully hidden
            
            if ((int8_t)sy >= hY) continue;

            // Calculate how many pixels of the cloud are above the horizon
            // If the cloud is 12 pixels tall, but only 5 are above hY, 
            // we clip the height to 3.

            uint8_t cloudHeight = 3; // Standardize this or use a lookup
            int8_t visibleHeight = hY - (int8_t)sy;
            
            if (visibleHeight > cloudHeight) visibleHeight = cloudHeight;
            if (visibleHeight <= 0) continue;

            const uint8_t* image;

            switch (i % 4) {

                case 0: 
                    image = Images::Cloud_00; 
                    break;

                case 1: 
                    image = Images::Cloud_01; 
                    break;

                case 2: 
                    image = Images::Cloud_02; 
                    break;

                default:
                    if (sx >= 0) { 
                        arduboy.drawPixel(sx, sy, WHITE);
                    }

                    continue;
            }

            drawClippedSprite(sx, sy, image, visibleHeight);

        }

    }

}


void drawClippedSprite(int16_t x, int16_t y, const uint8_t* bitmap, int8_t h) {

    if (h > 0) {
        arduboy.drawBitmap(x, y - 1, bitmap, 16, h, WHITE); 
    }

}


// ── drawEnemies ───────────────────────────────────────────────────────────
void drawEnemies() {

    uint8_t order[Constants::Entity_Count];

    // Precompute camera Z for all entities (for painter's sort)
    FP8 enemyZ[Constants::Entity_Count];

    for (uint8_t i = 0; i < Constants::Entity_Count; i++) {

        FP8 dx = enemies[i].getX() - player.getX();
        FP8 dz = enemies[i].getZ() - player.getZ();
        enemyZ[i] = -dx * player.getSinY() + dz * player.getCosY();

    }

    for (uint8_t i = 0; i < Constants::Entity_Count; i++) order[i] = i;

    // Insertion sort back-to-front by camera Z
    for (uint8_t i = 1; i < Constants::Entity_Count; i++) {

        uint8_t currentEnemyIdx = order[i];
        FP8 currentZ = enemyZ[currentEnemyIdx];

        uint8_t j = i;

        while (j > 0 && enemyZ[order[j - 1]] < currentZ) {
            order[j] = order[j - 1];
            j--;
        }

        order[j] = currentEnemyIdx;
    }


    // Render the planes back to front ..

    for (uint8_t k = 0; k < Constants::Entity_Count; k++) {

        uint8_t     i  = order[k];
        EnemyPlane &enemy = enemies[i];

        if (!enemy.isActive()) continue;

        FP8 dx = enemy.getX() - player.getX(); 
        FP8 dz = enemy.getZ() - player.getZ();
        Vec3        c  = worldToCameraFast(dx, dz, player.getCosY(), player.getSinY());
        ScreenPoint sp = project3d(c, enemy.getY(), player.getY(), player.getPitch());

        if (!sp.valid) continue;

        if (i == Constants::Heart_Idx) {

            if (enemy.getState() == AIState::Explode) {

                drawExplosion(sp.x, sp.y, constrain(sp.size, 1, 6), enemy.getExplosionFrame());

            } 
            else {

                uint8_t frame = (arduboy.frameCount / 8) % 6;
                uint8_t sz = constrain(sp.size, 1, 6);

                switch (sz) {

                    case 0 ... 2:
                        Sprites::drawOverwrite(sp.x - 4,  sp.y - 4, Images::HPInc_00, frame);
                        break;

                    case 3 ... 4:
                        Sprites::drawOverwrite(sp.x - 6,  sp.y - 8, Images::HPInc_01, frame);
                        break;

                    default:
                        Sprites::drawOverwrite(sp.x - 9,  sp.y - 8, Images::HPInc_02, frame);
                        break;

                }

            }
            continue;
        }

        FP8 vx  = fpSin(enemy.getYaw().getInteger());
        FP8 vz  = fpCos(enemy.getYaw().getInteger());
        FP8 dot = vx * (-player.getSinY()) + vz * player.getCosY();

        // Hysteresis: only flip sprite when dot clearly crosses ±0.2, preventing
        // flicker when enemy flies near-perpendicular to the camera.
        if      (dot < FP8(-0.2)) enemy.setHeadingToward(true);
        else if (dot > FP8( 0.2)) enemy.setHeadingToward(false);

        drawEnemySprite(arduboy, enemy, sp, enemy.getHeadingToward());

    }

}