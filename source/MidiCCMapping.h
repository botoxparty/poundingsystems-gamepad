#pragma once

namespace MidiCC {
    // Face Buttons (20-23)
    constexpr int A_BUTTON = 20;
    constexpr int B_BUTTON = 21;
    constexpr int X_BUTTON = 22;
    constexpr int Y_BUTTON = 23;

    // Shoulder Buttons (29-30)
    constexpr int L1_BUTTON = 29;
    constexpr int R1_BUTTON = 30;

    // D-Pad (31-34)
    constexpr int DPAD_UP = 31;
    constexpr int DPAD_DOWN = 32;
    constexpr int DPAD_LEFT = 33;
    constexpr int DPAD_RIGHT = 34;

    // Analog Sticks (1-4)
    constexpr int LEFT_STICK_X = 1;
    constexpr int LEFT_STICK_Y = 2;
    constexpr int RIGHT_STICK_X = 3;
    constexpr int RIGHT_STICK_Y = 4;

    // Triggers (5-6)
    constexpr int L2_TRIGGER = 5;
    constexpr int R2_TRIGGER = 6;

    // Touchpad (35-38)
    constexpr int TOUCHPAD_X = 35;
    constexpr int TOUCHPAD_Y = 36;
    constexpr int TOUCHPAD_PRESSURE = 37;
    constexpr int TOUCHPAD_BUTTON = 38;

    // Gyroscope (39-41)
    constexpr int GYRO_X = 39;
    constexpr int GYRO_Y = 40;
    constexpr int GYRO_Z = 41;

    // Accelerometer CCs (23-25)
    constexpr int ACCEL_X = 23;
    constexpr int ACCEL_Y = 24;
    constexpr int ACCEL_Z = 25;
} 