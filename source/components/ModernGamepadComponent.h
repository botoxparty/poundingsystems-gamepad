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
#include "Gyroscope.h"

class ModernGamepadComponent : public juce::Component,
                             private juce::Timer
{
public:
    ModernGamepadComponent(const GamepadManager::GamepadState& state);
    ~ModernGamepadComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void updateState(const GamepadManager::GamepadState& newState);

private:
    // Reference to gamepad state
    const GamepadManager::GamepadState& gamepadState;
    GamepadManager::GamepadState cachedState;
    bool midiLearnMode = false;

    // Child components
    ShoulderSection shoulderSection;
    DirectionalPad dPad;
    FaceButtons faceButtons;
    AnalogStick leftStick;
    AnalogStick rightStick;
    TouchPad touchPad;
    Gyroscope gyroscope;

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
    void sendMidiCC(int ccNumber, float value);
    void timerCallback() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModernGamepadComponent)
}; 