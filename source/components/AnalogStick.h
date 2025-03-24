#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_graphics/juce_graphics.h>
#include "ClassicButton.h"

class AnalogStick : public juce::Component
{
public:
    struct State {
        bool isEnabled = true;
        float xValue = 0.0f;    // -1.0 to 1.0
        float yValue = 0.0f;    // -1.0 to 1.0
        bool isPressed = false;
        int xCC = 0;
        int yCC = 0;
        int pressCC = 0;
        bool isLearnMode = false;
        juce::String name;
        bool isStick = true;
    };

    AnalogStick();
    explicit AnalogStick(const juce::String& name, bool isStick = true);
    ~AnalogStick() override = default;

    void setState(const State& newState);
    void resized() override;
    void paint(juce::Graphics& g) override;

    // Callbacks for button interactions
    std::function<void(const juce::String&)> onButtonClick;
    std::function<void(const juce::String&)> onLearnClick;
    std::function<void(const juce::String&, float)> onAxisChange;

    void setLearnMode(bool enabled) {
        state.isLearnMode = enabled;
        auto xProps = xButton.getProperties();
        xProps.isLearnMode = enabled;
        xButton.setProperties(xProps);
        
        auto yProps = yButton.getProperties();
        yProps.isLearnMode = enabled;
        yButton.setProperties(yProps);
        
        repaint();
    }

private:
    State state;

    // Buttons for MIDI learn
    ClassicButton xButton{ClassicButton::Properties{"X"}};
    ClassicButton yButton{ClassicButton::Properties{"Y"}};
    ClassicButton pressButton{ClassicButton::Properties{"Press"}};

    // Layout management
    juce::FlexBox layout;
    juce::FlexBox buttonLayout;

    // Visual properties
    float stickRadius = 20.0f;
    float deadZoneRadius = 5.0f;
    juce::Point<float> stickPosition;
    juce::Rectangle<float> stickBounds;

    void setupCallbacks();
    void updateStickPosition();
    void drawStick(juce::Graphics& g);
    void drawLabels(juce::Graphics& g);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AnalogStick)
}; 