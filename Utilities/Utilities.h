#ifndef UTILITIES_H
#define UTILITIES_H

class Utilities {
  public:
    static bool debounce(uint8_t switchPin, uint8_t debounceDelay = DEBOUNCE_DELAY);
    static void plot(String label, float value, bool final = false);
  private:
    static constexpr uint8_t DEBOUNCE_DELAY = 10; //ms
};

#endif