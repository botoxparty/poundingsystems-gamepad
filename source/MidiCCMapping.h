#pragma once

namespace MidiCC {
    // Analog Sticks (1-4)
    constexpr int LEFT_STICK_X = 1;
    constexpr int LEFT_STICK_Y = 2;
    constexpr int RIGHT_STICK_X = 3;
    constexpr int RIGHT_STICK_Y = 4;

    // Triggers (5-6)
    constexpr int L2_TRIGGER = 5;
    constexpr int R2_TRIGGER = 6;

    // Face Buttons (7-10)
    constexpr int A_BUTTON = 7;
    constexpr int B_BUTTON = 8;
    constexpr int X_BUTTON = 9;
    constexpr int Y_BUTTON = 10;

    // Select/Home/Cancel Buttons (11-13)
    constexpr int SELECT_BUTTON = 11;
    constexpr int HOME_BUTTON = 12;
    constexpr int CANCEL_BUTTON = 13;

    // Shoulder Buttons (14-15)
    constexpr int L1_BUTTON = 14;
    constexpr int R1_BUTTON = 15;

    // Stick Buttons (16-17)
    constexpr int LEFT_STICK_BUTTON = 16;
    constexpr int RIGHT_STICK_BUTTON = 17;

    // D-Pad (18-21)
    constexpr int DPAD_UP = 18;
    constexpr int DPAD_DOWN = 19;
    constexpr int DPAD_LEFT = 20;
    constexpr int DPAD_RIGHT = 21;

    // Touchpad (22-25)
    constexpr int TOUCHPAD_X = 22;
    constexpr int TOUCHPAD_Y = 23;
    constexpr int TOUCHPAD_PRESSURE = 24;
    constexpr int TOUCHPAD_BUTTON = 25;

    // Gyroscope (26-28)
    constexpr int GYRO_X = 26;
    constexpr int GYRO_Y = 27;
    constexpr int GYRO_Z = 28;

    // Accelerometer CCs (29-31)
    constexpr int ACCEL_X = 29;
    constexpr int ACCEL_Y = 30;
    constexpr int ACCEL_Z = 31;
} 