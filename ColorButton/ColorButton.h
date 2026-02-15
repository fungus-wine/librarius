/*
  ColorButton.h - Library for interfacing with an Adafruit Momentary Switch with LED color ring.
  Created by Damon Cali, November 28, 2019.
*/

#ifndef COLOR_BUTTON_H
#define COLOR_BUTTON_H

typedef enum {RED, ORANGE, YELLOW, GREEN, BLUE, INDIGO, VIOLET, WHITE, OFF} LedColor;

class ColorButton {
  public:
    ColorButton(uint8_t switchPin, uint8_t redPin, uint8_t greenPin, uint8_t bluePin);
    void red();
    void orange();
    void yellow();
    void green();
    void blue();
    void indigo();
    void violet();
    void white();
    void off();
    bool debounce();
  private:
    void illuminate(LedColor color);
    void setColors(uint8_t red, uint8_t green, uint8_t blue);
    static const uint8_t DEBOUNCE_DELAY = 10; //ms
    uint8_t *_switchPin;
    uint8_t *_redPin;
    uint8_t *_greenPin;
    uint8_t *_bluePin;
};

#endif
