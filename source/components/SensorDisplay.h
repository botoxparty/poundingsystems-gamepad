#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "ClassicButton.h"

/**
 * Component that displays gyroscope and accelerometer data with MIDI learn capability for each axis
 */
class SensorDisplay : public juce::Component {
public:
    struct State {
        bool enabled = false;
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;
        int xCC = -1;
        int yCC = -1;
        int zCC = -1;
        bool isLearnMode = false;
        bool isAccelerometer = false;  // true for accelerometer, false for gyroscope
    };

    SensorDisplay();
    ~SensorDisplay() override = default;

    void setState(const State& newState);
    const State& getState() const { return state; }

    // Callbacks for button interactions
    std::function<void(const juce::String&)> onButtonClick;
    std::function<void(const juce::String&)> onLearnClick;
    std::function<void(float, float, float)> onValueChange;

    void setLearnMode(bool enabled) {
        state.isLearnMode = enabled;
        auto xProps = xButton.getProperties();
        xProps.isLearnMode = enabled;
        xButton.setProperties(xProps);
        
        auto yProps = yButton.getProperties();
        yProps.isLearnMode = enabled;
        yButton.setProperties(yProps);
        
        auto zProps = zButton.getProperties();
        zProps.isLearnMode = enabled;
        zButton.setProperties(zProps);
        
        repaint();
    }

    // Component overrides
    void resized() override;
    void paint(juce::Graphics& g) override;

private:
    State state;

    // Buttons for each axis with MIDI learn
    ClassicButton xButton{{"X: 0.00", -1, false, false, juce::Colours::red.withAlpha(0.7f)}};
    ClassicButton yButton{{"Y: 0.00", -1, false, false, juce::Colours::green.withAlpha(0.7f)}};
    ClassicButton zButton{{"Z: 0.00", -1, false, false, juce::Colours::blue.withAlpha(0.7f)}};

    juce::FlexBox layout;

    void setupCallbacks();
    void updateButtonTexts();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SensorDisplay)
}; 