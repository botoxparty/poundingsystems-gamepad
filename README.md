# Gamepad MIDI

A standalone application that converts gamepad inputs to MIDI control messages, allowing you to use your gamepad as a MIDI controller in any DAW.

## Overview

This application uses SDL3 to detect and read gamepad inputs, and converts them to MIDI control change (CC) messages that can be used in any DAW or MIDI-compatible software.

Key features:
- Support for up to 4 gamepads simultaneously
- Converts gamepad axes to MIDI CC values
- Converts gamepad buttons to MIDI CC values
- Visual representation of gamepad state
- Compatible with most game controllers (Xbox, PlayStation, etc.)
- Select any MIDI output device

## How It Works

1. Launch the application
2. Select your MIDI output device from the dropdown
3. Connect your gamepad(s)
4. The application will automatically convert gamepad inputs to MIDI CC messages
5. Use your DAW's MIDI learn functionality to map the CC messages to parameters

## MIDI Mapping

The application uses the following default MIDI mappings (all on MIDI Channel 1):

### Axes (CC 1-6)
- Left Stick X: CC 1
- Left Stick Y: CC 2
- Right Stick X: CC 3
- Right Stick Y: CC 4
- Left Trigger: CC 5
- Right Trigger: CC 6

### Buttons (CC 20-34)
- A: CC 20
- B: CC 21
- X: CC 22
- Y: CC 23
- Back: CC 24
- Guide: CC 25
- Start: CC 26
- Left Stick (L3): CC 27
- Right Stick (R3): CC 28
- Left Shoulder (L1): CC 29
- Right Shoulder (R1): CC 30
- DPad Up: CC 31
- DPad Down: CC 32
- DPad Left: CC 33
- DPad Right: CC 34

All axes are normalized to the MIDI CC range (0-127), and buttons send 127 when pressed and 0 when released.

## Usage Examples

1. **Control a synthesizer**: Map gamepad axes to synth parameters like filter cutoff, resonance, etc.
2. **Control mixer faders**: Map gamepad axes to track volume or panning
3. **Trigger samples**: Map gamepad buttons to trigger samples or loops
4. **Control effects**: Map gamepad axes to effect parameters like delay time, reverb size, etc.
5. **Transport control**: Map gamepad buttons to play, pause, record, etc.

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
