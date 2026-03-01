#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

#include <stdint.h>

// Arduino Giga R1 WiFi board-level configuration.
// Built-in LEDs (LEDR, LEDG, LEDB) are active-low on this board.

namespace Config {
    constexpr uint32_t SERIAL_BAUD_RATE = 115200;
    constexpr uint16_t USB_ENUM_DELAY_MS = 150;
    constexpr uint32_t CAN_BUS_SPEED = 250000;
}

#endif // BOARD_CONFIG_H
