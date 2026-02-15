/*
  ColorButton.cpp - Library for interfacing with an Adafruit Momentary Switch with LED color ring.
  Created by Damon Cali, November 28, 2019.
*/

#include "Arduino.h"
#include "ColorButton.h"

ColorButton::ColorButton(uint8_t switchPin, uint8_t redPin, uint8_t greenPin, uint8_t bluePin) {
  _switchPin = switchPin;
  _redPin = redPin;
  _greenPin = greenPin;
  _bluePin = bluePin;

  pinMode(_redPin, OUTPUT);
  pinMode(_greenPin, OUTPUT);
  pinMode(_bluePin, OUTPUT);
  pinMode(_switchPin, INPUT);
  digitalWrite(_switchPin, HIGH);
}

void ColorButton::off() {
  illuminate(OFF);
}
void ColorButton::red() {
  illuminate(RED);
}
void ColorButton::orange() {
  illuminate(ORANGE);
}
void ColorButton::yellow() {
  illuminate(YELLOW);
}
void ColorButton::green() {
  illuminate(GREEN);
}
void ColorButton::blue() {
  illuminate(BLUE);
}
void ColorButton::indigo() {
  illuminate(INDIGO);
}
void ColorButton::violet() {
  illuminate(VIOLET);
}
void ColorButton::white() {
  illuminate(WHITE);
}

void ColorButton::illuminate(LedColor color) {
  switch(color) {
    case RED :
      setColors(255,0,0);
      break;
    case ORANGE :
      setColors(255, 40, 0);
      break;
    case YELLOW :
      setColors(255,255,0);
      break;
    case GREEN :
      setColors(0,255,0);
      break;
    case BLUE : 
      setColors(0,0,255);
      break;
    case INDIGO :
      setColors(63,0,255);
      break;
    case VIOLET :
      setColors(127,0,255);
      break;
    case WHITE :
      setColors(255,255,255);
      break;
    case OFF :
      setColors(0,0,0);
      break;
  }
}

void ColorButton::setColors(uint8_t red, uint8_t green, uint8_t blue) {
  analogWrite(_redPin, 255 - red);
  analogWrite(_greenPin, 255 - green);
  analogWrite(_bluePin, 255 - blue);
}

bool ColorButton::debounce() { 
  bool newState;
  bool oldState; 
  oldState = digitalRead(_switchPin);
  for(int i = 0; i < DEBOUNCE_DELAY; i++) {
    delay(1);
    newState = digitalRead(_switchPin);
    if( newState != oldState) {
      i = 0;
      oldState = newState;
    } 
  } 
  return newState;
}