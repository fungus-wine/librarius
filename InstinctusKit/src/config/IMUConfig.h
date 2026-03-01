#ifndef IMU_CONFIG_H
#define IMU_CONFIG_H

#include <stdint.h>
#include "CoordinateTransform.h"

// ICM20948 IMU configuration.
// Ranges/rates are plain ints — ICM20948Interface maps them to library enums.

namespace Config {
    constexpr uint8_t  IMU_I2C_ADDRESS       = 0x69;
    constexpr uint8_t  IMU_ACCEL_RANGE_G     = 4;
    constexpr uint16_t IMU_GYRO_RANGE_DPS    = 500;
    constexpr uint16_t IMU_ACCEL_RATE_DIVISOR = 4095;
    constexpr uint8_t  IMU_GYRO_RATE_DIVISOR  = 255;
    constexpr uint8_t  IMU_MAG_RATE_HZ       = 10;

    // ICM20948 physical axes: X=forward, Y=right, Z=down
    // Robot frame:            X=forward, Y=left,  Z=up
    constexpr CoordinateTransform BALANCE_IMU_TRANSFORM = {
        {0, +1.0f},   // robot X = +imu X  (forward, unchanged)
        {1, -1.0f},   // robot Y = -imu Y  (right -> left)
        {2, -1.0f},   // robot Z = -imu Z  (down  -> up)
    };
}

#endif // IMU_CONFIG_H
