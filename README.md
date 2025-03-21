![PAMPLEJUCE](assets/images/pamplejuce.png)
[![](https://github.com/sudara/pamplejuce/actions/workflows/build_and_test.yml/badge.svg)](https://github.com/sudara/pamplejuce/actions)

Pamplejuce is a ~~template~~ lifestyle for creating and building JUCE plugins in 2024.

Out of the box, it:

1. Supports C++20.
2. Uses JUCE 8.x as a git submodule (tracking develop).
3. Relies on CMake 3.24.1 and higher for cross-platform building.
4. Has [Catch2](https://github.com/catchorg/Catch2) v3.7.0 for the test framework and runner.
5. Includes a `Tests` target and a `Benchmarks` target some examples to get started quickly.
6. Has [Melatonin Inspector](https://github.com/sudara/melatonin_inspector) installed as a JUCE module to help relieve headaches when building plugin UI.

It also has integration with GitHub Actions, specifically:

1. Building and testing cross-platform (linux, macOS, Windows) binaries
2. Running tests and benchmarks in CI
3. Running [pluginval](http://github.com/tracktion/pluginval) 1.x against the binaries for plugin validation
4. Config for [installing Intel IPP](https://www.intel.com/content/www/us/en/developer/tools/oneapi/ipp.html)
5. [Code signing and notarization on macOS](https://melatonin.dev/blog/how-to-code-sign-and-notarize-macos-audio-plugins-in-ci/)
6. [Windows code signing via Azure Trusted Signing](https://melatonin.dev/blog/code-signing-on-windows-with-azure-trusted-signing/)

It also contains:

1. A `.gitignore` for all platforms.
2. A `.clang-format` file for keeping code tidy.
3. A `VERSION` file that will propagate through JUCE and your app.
4. A ton of useful comments and options around the CMake config.

## How does this all work at a high level?

Check out the [official Pamplejuce documentation](https://melatonin.dev/manuals/pamplejuce/how-does-this-all-work/).

[![Arc - 2024-10-01 51@2x](https://github.com/user-attachments/assets/01d19d2d-fbac-481f-8cec-e9325b2abe57)](https://melatonin.dev/manuals/pamplejuce/how-does-this-all-work/)

## Setting up for YOUR project

This is a template repo!

That means the easiest thing to do is click "[Use this template](https://github.com/sudara/pamplejuce/generate)" here or at the top of the page to get your own repo with all the code here.

Then check out the [documentation](https://melatonin.dev/manuals/pamplejuce/setting-your-project-up/) so you know what to tweak.

## Having Issues?

Thanks to everyone who has contributed to the repository. 

This repository covers a _lot_ of ground. JUCE itself has a lot of surface area. It's a group effort to maintain the garden and keep things nice!

If something isn't just working out of the box — *it's probably not just you* — others are running into the problem, too, I promise. Check out [the official docs](https://melatonin.dev/manuals/pamplejuce), then please do [open an issue](https://github.com/sudara/pamplejuce/issues/new)!

# Gamepad MIDI Controller

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
git clone https://github.com/yourusername/gamepad-midi-controller.git
cd gamepad-midi-controller

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
