#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "GamepadManager.h"
#include "MidiOutputManager.h"
#include "GamepadComponent.h"
#include "BinaryData.h"

class StandaloneApp : public juce::Component,
                      private juce::Timer
{
public:
    StandaloneApp();
    ~StandaloneApp() override;
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    
private:
    void timerCallback() override;
    void handleGamepadStateChange();
    void setupMidiMappings();
    
    // State tracking for single gamepad
    struct GamepadState {
        float axes[GamepadManager::MAX_AXES] = {};
        bool buttons[GamepadManager::MAX_BUTTONS] = {};
        bool connected = false;
    };
    GamepadState previousGamepadState;
    
    // UI Components
    std::unique_ptr<GamepadComponent> gamepadComponent;
    juce::ImageComponent logoComponent;
    
    // Managers
    GamepadManager gamepadManager;
    
    // MIDI mapping configuration
    struct MidiMapping {
        int channel;
        int ccNumber;
        float minValue;
        float maxValue;
        bool isButton;
    };
    
    std::array<MidiMapping, GamepadManager::MAX_AXES> axisMappings;
    std::array<MidiMapping, GamepadManager::MAX_BUTTONS> buttonMappings;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StandaloneApp)
}; 