#ifndef MOTOR_CONFIG_H
#define MOTOR_CONFIG_H

#include <stdint.h>

// ODrive S1 motor controller configuration.
// Placeholder — will grow as motor control is implemented.

namespace Config {
    struct MotorInstanceConfig {
        uint8_t canId;
    };

    constexpr MotorInstanceConfig MOTOR_LEFT  = { 0x01 };
    constexpr MotorInstanceConfig MOTOR_RIGHT = { 0x02 };
}

#endif // MOTOR_CONFIG_H
