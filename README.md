# Gamepad MIDI

A standalone application that converts gamepad inputs to MIDI control messages, allowing you to use your gamepad as a MIDI controller in any DAW.

## Overview

This application uses SDL3 to detect and read gamepad inputs, and converts them to MIDI control change (CC) messages that can be used in any DAW or MIDI-compatible software.

Key features:
- Support for modern gamepads with advanced features
- Converts gamepad axes, buttons, triggers, touchpad, and motion sensors to MIDI CC values
- Visual representation of gamepad state with a modern UI
- MIDI Learn mode for easy mapping
- Automatic virtual MIDI device creation (MacOS, Linux only. NOT available on Windows)
- Compatible with most game controllers (Xbox, PlayStation, etc.)
- Select any MIDI output device
- Real-time visual feedback of all inputs

## How It Works

1. Launch the application
2. The application automatically creates a virtual MIDI device named "Gamepad MIDI"
3. Select your preferred MIDI output device from the dropdown
4. Connect your gamepad
5. Use the Learn Mode button to easily map controls to your DAW parameters
6. The application will automatically convert gamepad inputs to MIDI CC messages

## MIDI Mapping

The application supports the following inputs (all on MIDI Channel 1):

### Analog Sticks
- Left Stick X: CC 1
- Left Stick Y: CC 2
- Right Stick X: CC 3
- Right Stick Y: CC 4
- Left Stick Press (L3): CC 27
- Right Stick Press (R3): CC 28

### Triggers and Shoulders
- Left Trigger: CC 5
- Right Trigger: CC 6
- Left Shoulder (L1): CC 29
- Right Shoulder (R1): CC 30

### Face Buttons
- A: CC 20
- B: CC 21
- X: CC 22
- Y: CC 23

### D-Pad
- Up: CC 31
- Down: CC 32
- Left: CC 33
- Right: CC 34

### Touchpad (if supported by gamepad)
- X Position: CC 35
- Y Position: CC 36
- Pressure: CC 37
- Touchpad Button: CC 38

### Motion Sensors (if supported by gamepad)
#### Gyroscope
- X Axis: CC 39
- Y Axis: CC 40
- Z Axis: CC 41

#### Accelerometer
- X Axis: CC 42
- Y Axis: CC 43
- Z Axis: CC 44

All axes are normalized to the MIDI CC range (0-127), and buttons send 127 when pressed and 0 when released.

## Usage Examples

1. **Control a synthesizer**: Map gamepad axes to synth parameters like filter cutoff, resonance, etc.
2. **Control mixer faders**: Map gamepad axes to track volume or panning
3. **Trigger samples**: Map gamepad buttons to trigger samples or loops
4. **Control effects**: Map gamepad axes to effect parameters like delay time, reverb size, etc.
5. **Motion control**: Use the gyroscope and accelerometer for expressive control of parameters
6. **Touch control**: Use the touchpad for precise parameter control

## Building From Source

This project uses CMake for building:

```bash
# Clone the repository
git clone https://github.com/botoxparty/poundingsystems-gamepad.git
cd poundingsystems-gamepad

# Initialize submodules
git submodule update --init --recursive

# Create build directory
mkdir build
cd build

# Configure and build
cmake ..
cmake --build .
```

## Requirements

- C++20 compatible compiler
- JUCE framework (included as a submodule)
- SDL3 (added automatically through CMake FetchContent)

## Supported Software

This application works with any software that can receive MIDI CC messages, including:
- Ableton Live
- Logic Pro
- FL Studio
- Bitwig Studio
- Reaper
- Pro Tools
- Cubase
- Studio One
- VCV Rack
- Reason
- And many more...

## License

This project is licensed under the terms of the GPL v3 license.

## Credits

- Built with [JUCE](https://juce.com/)
- Gamepad input powered by [SDL3](https://www.libsdl.org/)
- Developed by Pounding Systems
