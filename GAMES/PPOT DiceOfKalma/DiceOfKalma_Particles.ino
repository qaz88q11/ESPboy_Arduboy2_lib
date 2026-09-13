#include "src/utils/Arduboy2Ext.h"
#include "src/utils/Utils.h"
#include "src/utils/Constants.h"

void launchParticles() {

    launchParticles(32, 64);

}

void launchParticles(int16_t x, int16_t y) {

    for (uint8_t i = 0; i < Constants::ParticlesMax; i++) {

        if (!particles[i].render() ) {
            particles[i].setX(x);
            particles[i].setY(y);
            particles[i].setVelX(random(-4, 5));
            particles[i].setVelY(random(-4, 4));
            particles[i].setCounter(random(20, 56));

        }
    
    }

}

void updateAndRenderParticles() {

    for (uint8_t i = 0; i < Constants::ParticlesMax; i++) {

        particles[i].update();

        if ( particles[i].render() ) {

            Sprites::drawExternalMask(particles[i].getY() - 1, particles[i].getX() - 1, Images::Particle_Sml, Images::Particle_Sml_Mask, 0, 0);

        }

    }

}
