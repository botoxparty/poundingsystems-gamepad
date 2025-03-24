#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "GamepadManager.h"
#include "MidiOutputManager.h"

/**
 * Component that visualizes a gamepad's state with classic Windows 98/2000 style
 */
class GamepadComponent : public juce::Component, 
                         private juce::Timer
{
public:
    GamepadComponent(const GamepadManager::GamepadState& state);
    ~GamepadComponent() override;
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    
    // Update the component to display the current state
    void updateState(const GamepadManager::GamepadState& newState);
    
    // MIDI Learn mode control
    void setMidiLearnMode(bool enabled);
    bool isMidiLearnMode() const { return midiLearnMode; }
    
private:
    void timerCallback() override;
    void mouseDown(const juce::MouseEvent& event) override;
    
    // Helper methods for drawing classic Windows style UI elements
    void drawClassicButton(juce::Graphics& g, const juce::Rectangle<float>& bounds, bool isPressed, const juce::String& text = "", int ccNumber = -1);
    void drawClassicInsetPanel(juce::Graphics& g, const juce::Rectangle<float>& bounds);
    void drawClassicGroupBox(juce::Graphics& g, const juce::Rectangle<float>& bounds, const juce::String& text);
    
    // Send MIDI CC messages for gyroscope and touchpad
    void sendGyroscopeMidiCC(const GamepadManager::GamepadState::GyroscopeState& gyro);
    void sendTouchpadMidiCC(const GamepadManager::GamepadState::TouchpadState& touchpad);
    void sendMidiLearnCC(int ccNumber, float value);
    
    // Reference to the gamepad state to visualize
    const GamepadManager::GamepadState& gamepadState;
    
    // Cached gamepad state for UI updates
    GamepadManager::GamepadState cachedState;
    
    // MIDI Learn mode state
    bool midiLearnMode = false;
    juce::Rectangle<float> midiLearnButtonBounds;
    
    // UI elements for button visualization
    struct ButtonVisual
    {
        juce::String name;
        juce::Rectangle<float> bounds;
        int buttonIndex;
    };
    
    // UI elements for axis visualization
    struct AxisVisual
    {
        juce::String name;
        juce::Rectangle<float> bounds;
        int axisIndex;
        bool isStick;  // True for joysticks, false for triggers
    };
    
    // MIDI Learn buttons for continuous controls
    struct MidiLearnControl
    {
        juce::String name;
        juce::Rectangle<float> bounds;
        int ccNumber;
        bool isAxis;  // true for continuous controls, false for buttons
        int index;    // axis or button index
        int subIndex; // for multi-axis controls like sticks (0=X, 1=Y)
    };
    
    // All the button visuals
    std::vector<ButtonVisual> buttonVisuals;
    
    // All the axis visuals
    std::vector<AxisVisual> axisVisuals;
    
    // All MIDI learn controls
    std::vector<MidiLearnControl> midiLearnControls;
    
    // Setup the visual elements
    void setupVisuals();
    void setupMidiLearnControls();
    
    // Helper to find MIDI learn control under point
    MidiLearnControl* findMidiLearnControlAt(juce::Point<float> point);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GamepadComponent)
}; 