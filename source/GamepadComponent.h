#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "GamepadManager.h"

/**
 * Component that visualizes a gamepad's state
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
    
private:
    void timerCallback() override;
    
    // Reference to the gamepad state to visualize
    const GamepadManager::GamepadState& gamepadState;
    
    // Cached gamepad state for UI updates
    GamepadManager::GamepadState cachedState;
    
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
    
    // All the button visuals
    std::vector<ButtonVisual> buttonVisuals;
    
    // All the axis visuals
    std::vector<AxisVisual> axisVisuals;
    
    // Setup the visual elements
    void setupVisuals();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GamepadComponent)
}; 