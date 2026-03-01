#ifndef COORDINATE_TRANSFORM_H
#define COORDINATE_TRANSFORM_H

// ============================================================================
// COORDINATE TRANSFORM
// ============================================================================
// Sensors are rarely mounted with their axes aligned to the robot's frame.
// This module provides a lightweight, zero-cost way to remap any sensor's
// raw (X, Y, Z) readings into Calvin's canonical robot frame:
//
//   Robot frame:  X = forward,  Y = left,  Z = up
//   Origin:       wheel axle center
//
// How it works:
//   Each AxisMap says "robot axis N comes from sensor axis `axis`, scaled
//   by `sign`."  A CoordinateTransform holds three AxisMaps — one per
//   robot axis.  applyTransform() applies all three in a single call.
//
// Example — an IMU mounted with its Y pointing forward, X pointing right,
// and Z pointing up:
//
//   constexpr CoordinateTransform IMU_TRANSFORM = {
//       {1, +1.0f},   // robot X (forward) = +sensor Y
//       {0, -1.0f},   // robot Y (left)    = -sensor X  (right -> left)
//       {2, +1.0f},   // robot Z (up)      = +sensor Z  (unchanged)
//   };
//
// The actual IMU transform for Calvin is defined in IMUConfig.h.

// Maps one robot axis to one sensor axis.
//   axis: index into the sensor's raw output array (0=X, 1=Y, 2=Z)
//   sign: +1.0 or -1.0 to preserve or invert polarity
struct AxisMap { int axis; float sign; };

// A complete 3-axis remapping from sensor frame to robot frame.
// Each member describes which sensor axis (and polarity) produces
// the corresponding robot axis.
struct CoordinateTransform {
    AxisMap x;  // sensor axis & sign that produce robot X (forward)
    AxisMap y;  // sensor axis & sign that produce robot Y (left)
    AxisMap z;  // sensor axis & sign that produce robot Z (up)
};

// Apply a coordinate transform to a sensor reading.
//   (sx, sy, sz): raw sensor values in the sensor's native frame
//   (rx, ry, rz): output values in the robot's canonical frame
inline void applyTransform(const CoordinateTransform& t,
                           float sx, float sy, float sz,
                           float& rx, float& ry, float& rz) {
    float src[3] = {sx, sy, sz};
    rx = t.x.sign * src[t.x.axis];
    ry = t.y.sign * src[t.y.axis];
    rz = t.z.sign * src[t.z.axis];
}

#endif // COORDINATE_TRANSFORM_H
