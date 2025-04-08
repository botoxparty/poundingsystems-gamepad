#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_graphics/juce_graphics.h>
#include <juce_core/juce_core.h>

#include "GamepadManager.h"
#include "MidiOutputManager.h"
#include "ShoulderSection.h"
#include "DirectionalPad.h"
#include "FaceButtons.h"
#include "AnalogStick.h"
#include "TouchPad.h"
#include "SensorDisplay.h"

class StandaloneApp;  // Forward declaration

class ModernGamepadComponent : public juce::Component,
                             private juce::Timer
{
public:
    ModernGamepadComponent(const GamepadManager::GamepadState& state, StandaloneApp& app);
    ~ModernGamepadComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void updateState(const GamepadManager::GamepadState& newState);
    bool isMidiLearnMode() const { return midiLearnMode; }
    
    // Call this when MIDI mappings have been updated
    void midiMappingsChanged() { updateState(gamepadState); }

private:
    // Reference to gamepad state and app
    const GamepadManager::GamepadState& gamepadState;
    StandaloneApp& app;
    GamepadManager::GamepadState cachedState;
    bool midiLearnMode = false;

    // Child components
    ShoulderSection shoulderSection;
    DirectionalPad dPad;
    FaceButtons faceButtons;
    AnalogStick leftStick;
    AnalogStick rightStick;
    TouchPad touchPad;
    SensorDisplay gyroscopeDisplay;
    SensorDisplay accelerometerDisplay;

    // Layout management
    juce::FlexBox mainLayout;
    juce::FlexBox topRow;
    juce::FlexBox middleRow;
    juce::FlexBox bottomRow;

    // UI Elements
    juce::TextButton learnModeButton;
    juce::Label statusLabel;

    // Helper methods
    void setupComponents();
    void setupLayout();
    void setupCallbacks();
    void setMidiLearnMode(bool enabled);
    void sendMidiCC(int controlIndex, float value, bool isButton);
    void timerCallback() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModernGamepadComponent)
}; 