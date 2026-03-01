#ifndef TOF_CONFIG_H
#define TOF_CONFIG_H

#include <stdint.h>

// VL53L4CX Time-of-Flight sensor configuration.
// Per-instance structs for front and rear sensors.

namespace Config {
    struct ToFInstanceConfig {
        int      xshutPin;
        uint8_t  i2cAddress;
        float    warnDistanceMm;
        uint32_t timingBudgetUs;
    };

    constexpr ToFInstanceConfig TOF_REAR  = { 25, 0x30, 20.0f, 33000 };
    constexpr ToFInstanceConfig TOF_FRONT = { 23, 0x29, 20.0f, 33000 };

    constexpr float NO_TARGET_DISTANCE = 9999.0f;
}

#endif // TOF_CONFIG_H
