#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

namespace Pins {
    // Motor pins
    // constexpr uint8_t LEFT_MOTOR_PIN1 = 5;
    // constexpr uint8_t LEFT_MOTOR_PIN2 = 6;
    // constexpr uint8_t RIGHT_MOTOR_PIN1 = 9;
    // constexpr uint8_t RIGHT_MOTOR_PIN2 = 10;
    // constexpr uint8_t MOTOR_ENABLE_PIN = 8;

    // // Sensor pins
    // constexpr uint8_t ULTRASONIC_TRIG_PIN = 2;
    // constexpr uint8_t ULTRASONIC_ECHO_PIN = 3;
    // constexpr uint8_t LINE_SENSOR_LEFT_PIN = A0;
    // constexpr uint8_t LINE_SENSOR_CENTER_PIN = A1;
    // constexpr uint8_t LINE_SENSOR_RIGHT_PIN = A2;
    // constexpr uint8_t BATTERY_VOLTAGE_PIN = A3;
}

namespace System {
    constexpr uint8_t MAIN_LOOP_DELAY = 1;    // Milliseconds between main loop iterations
    constexpr uint8_t SENSOR_UPDATE_INTERVAL = 50;   // Milliseconds between sensor readings
}

// // PID control
// constexpr float KP 2.0                 // Proportional gain
// constexpr float KI 0.1                 // Integral gain
// constexpr float KD 1.0                 // Derivative gain

#endif // CONFIG_H