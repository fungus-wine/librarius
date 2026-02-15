#ifndef HARDWARE_CONSTANTS_H
#define HARDWARE_CONSTANTS_H

#include <Adafruit_ICM20X.h>
#include <Adafruit_ICM20948.h>
#include <Adafruit_Sensor.h>

namespace Hardware {
    // ICM20948 IMU Constants - All in one place
    constexpr uint8_t                ICM20948_IC2_ADDRRESS = 0x68;
    constexpr icm20948_accel_range_t ICM20948_ACCEL_RANGE = ICM20948_ACCEL_RANGE_16_G;
    constexpr icm20948_gyro_range_t  ICM20948_GYRO_RANGE = ICM20948_GYRO_RANGE_2000_DPS;
    constexpr uint16_t               ICM20948_ACCEL_RATE_DIVISOR = 4095;
    constexpr uint8_t                ICM20948_GYRO_RATE_DIVISOR = 255;
    constexpr ak09916_data_rate_t    ICM20948_MAG_DATARATE = AK09916_MAG_DATARATE_10_HZ;
}

#endif // HARDWARE_CONSTANTS_H