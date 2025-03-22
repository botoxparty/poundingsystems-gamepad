#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "GamepadManager.h"
#include "MidiOutputManager.h"
#include "GamepadComponent.h"
#include "StatusBar.h"
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
    
    // State tracking
    struct GamepadState {
        float axes[GamepadManager::MAX_AXES] = {};
        bool buttons[GamepadManager::MAX_BUTTONS] = {};
        bool connected = false;
    };
    std::array<GamepadState, GamepadManager::MAX_GAMEPADS> previousGamepadStates;
    
    // UI Components
    StatusBar statusBar;
    std::array<std::unique_ptr<GamepadComponent>, GamepadManager::MAX_GAMEPADS> gamepadComponents;
    juce::ImageComponent logoComponent;
    
    // Managers
    GamepadManager gamepadManager;
    MidiOutputManager midiOutput;
    
    // MIDI mapping configuration
    struct MidiMapping {
        int channel;
        int ccNumber;
        float minValue;
        float maxValue;
        bool isButton;
    };
    
    std::array<std::array<MidiMapping, GamepadManager::MAX_AXES>, GamepadManager::MAX_GAMEPADS> axisMappings;
    std::array<std::array<MidiMapping, GamepadManager::MAX_BUTTONS>, GamepadManager::MAX_GAMEPADS> buttonMappings;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StandaloneApp)
}; 