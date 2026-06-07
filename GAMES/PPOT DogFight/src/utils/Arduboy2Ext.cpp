#include "Arduboy2Ext.h"

Arduboy2Ext::Arduboy2Ext() : Arduboy2Base() { }

//uint16_t rnd = 0xACE1;

uint8_t Arduboy2Ext::justPressedButtons() const {

    return (~previousButtonState & currentButtonState);

}

uint8_t Arduboy2Ext::pressedButtons() const {

    return currentButtonState;

}

void Arduboy2Ext::clearButtonState() {

    currentButtonState = previousButtonState = 0;

}


void Arduboy2Ext::resetFrameCount() {

    frameCount = 0;

}

uint8_t Arduboy2Ext::getFrameCount() const {

    return frameCount;

}

uint8_t Arduboy2Ext::getFrameCount(uint8_t mod) const {

    return frameCount % mod;

}

bool Arduboy2Ext::getFrameCountHalf(uint8_t mod) const {

	return getFrameCount(mod) > (mod / 2);

}

bool Arduboy2Ext::isFrameCount(uint8_t mod) const {

    return (frameCount % mod) == 0;

}

bool Arduboy2Ext::isFrameCount(uint8_t mod, uint8_t val) const {

    return (frameCount % mod) == val;

}


void Arduboy2Ext::displayClearToWhite() {

    display(true);
    memset(sBuffer, 0xff, 1024);

}


/* ----------------------------------------------------------------------------
 *  Draw a horizontal dotted line. 
 */
void Arduboy2Ext::drawHorizontalDottedLine(uint8_t x1, uint8_t x2, uint8_t y, uint8_t colour) {

	uint8_t diff = (x2 - x1);

	for (uint8_t x = 0; x <= diff; x += 2) {

		drawPixel(x1 + x, y, colour);

    }

}


/* ----------------------------------------------------------------------------
 *  Draw a vertical dotted line. 
 */
void Arduboy2Ext::drawVerticalDottedLine(uint8_t y1, uint8_t y2, uint8_t x, uint8_t colour) {

	uint8_t diff = (y2 - y1);

	for (uint8_t y = 0; y <= diff; y += 2) {

		drawPixel(x, y1 + y, colour);

    }
  
}


void Arduboy2Ext::initRandomLFSRSeed() {
    // adapted from Arduboy2 library
    //power_adc_enable(); // ADC on

    // do an ADC read from an unconnected input pin
    //ADCSRA |= _BV(ADSC); // start conversion (ADMUX has been pre-set in boot())
    //while (bit_is_set(ADCSRA, ADSC)) { } // wait for conversion complete

    //rnd = ADC ^ (uint16_t)micros();
    //power_adc_disable(); // ADC off

}

uint8_t Arduboy2Ext::randomLFSR(uint8_t min, uint8_t max) {
    //uint16_t r = rnd;
    //r ^= TCNT0; // add some extra timing randomness
    //(r & 1) ? r = (r >> 1) ^ 0xB400 : r >>= 1;
    //rnd = r;
    //return r % (max - min) + min;
    return random(min, max);
}