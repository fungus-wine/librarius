#include <Arduino.h>
#include "Utilities.h"

bool Utilities::debounce(uint8_t switchPin, uint8_t debounceDelay) { 
  bool newState;
  bool oldState; 
  oldState = digitalRead(switchPin);
  for(int i = 0; i < debounceDelay; i++) {
    delay(1);
    newState = digitalRead(switchPin);
    if( newState != oldState) {
      i = 0;
      oldState = newState;
    } 
  } 
  return newState;
}

void Utilities::plot(String label, float value, bool final) {
  Serial.print(label);
  Serial.print(':');
  Serial.print(value);
  if (final==false) {
    Serial.print(",");
  } else {
    Serial.println();
  }
}
