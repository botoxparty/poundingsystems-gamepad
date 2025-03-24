#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_graphics/juce_graphics.h>
#include "ClassicButton.h"

class TouchPad : public juce::Component
{
public:
    struct State {
        bool isEnabled = true;
        float xValue = 0.0f;    // 0.0 to 1.0
        float yValue = 0.0f;    // 0.0 to 1.0
        float pressure = 0.0f;  // 0.0 to 1.0
        bool isPressed = false;
        int xCC = 0;
        int yCC = 0;
        int pressureCC = 0;
        bool isLearnMode = false;
    };

    TouchPad();
    ~TouchPad() override = default;

    void setState(const State& newState);
    void resized() override;
    void paint(juce::Graphics& g) override;

    // Callbacks for button interactions
    std::function<void(const juce::String&)> onButtonClick;
    std::function<void(const juce::String&)> onLearnClick;

private:
    State state;

    // Buttons for MIDI learn
    ClassicButton xButton{ClassicButton::Properties{"X"}};
    ClassicButton yButton{ClassicButton::Properties{"Y"}};
    ClassicButton pressureButton{ClassicButton::Properties{"Pressure"}};

    // Layout management
    juce::FlexBox layout;
    juce::FlexBox buttonLayout;

    // Visual properties
    juce::Rectangle<float> touchArea;
    juce::Point<float> touchPosition;
    float touchRadius = 10.0f;
    float pressureBarHeight = 20.0f;

    void setupCallbacks();
    void updateTouchPosition();
    void drawTouchArea(juce::Graphics& g);
    void drawPressureBar(juce::Graphics& g);
    void drawLabels(juce::Graphics& g);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TouchPad)
}; 