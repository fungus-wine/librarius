#ifndef BALANCE_CONFIG_H
#define BALANCE_CONFIG_H

#include <stdint.h>

// Balance control and timing configuration.

namespace Config {
    // Complementary filter
    constexpr float TILT_ALPHA = 0.98f;

    // Observer notification threshold (degrees)
    constexpr float TILT_CHANGE_THRESHOLD = 1.0f;

    // Safety limit (degrees)
    constexpr float EMERGENCY_TILT_ANGLE = 45.0f;

    // M4 loop timing
    constexpr uint8_t M4_LOOP_DELAY_MS = 9;

    // Telemetry intervals
    constexpr uint16_t IMU_DATA_INTERVAL_MS     = 500;
    constexpr uint16_t TOF_DATA_INTERVAL_MS     = 500;
    constexpr uint16_t QUEUE_STATUS_INTERVAL_MS = 5000;
}

#endif // BALANCE_CONFIG_H
