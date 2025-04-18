#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "ClassicButton.h"

/**
 * Component that displays face buttons (A/B/X/Y) with MIDI learn capability
 */
class FaceButtons : public juce::Component {
public:
    struct State {
        int aCC = -1;
        int bCC = -1;
        int xCC = -1;
        int yCC = -1;
        bool aPressed = false;
        bool bPressed = false;
        bool xPressed = false;
        bool yPressed = false;
        bool isLearnMode = false;
    };

    FaceButtons();
    ~FaceButtons() override = default;

    void setState(const State& newState);
    const State& getState() const { return state; }

    void setLearnMode(bool enabled) {
        state.isLearnMode = enabled;
        auto aProps = aButton.getProperties();
        aProps.isLearnMode = enabled;
        aButton.setProperties(aProps);
        
        auto bProps = bButton.getProperties();
        bProps.isLearnMode = enabled;
        bButton.setProperties(bProps);
        
        auto xProps = xButton.getProperties();
        xProps.isLearnMode = enabled;
        xButton.setProperties(xProps);
        
        auto yProps = yButton.getProperties();
        yProps.isLearnMode = enabled;
        yButton.setProperties(yProps);
        
        repaint();
    }

    // Event callbacks
    std::function<void(const juce::String&, float)> onButtonStateChanged;  // Called when button state changes (1.0f for press, 0.0f for release)

    // Component override
    void resized() override;
    void paint(juce::Graphics& g) override;

private:
    State state;
    ClassicButton aButton{ClassicButton::Properties::create("A")};
    ClassicButton bButton{ClassicButton::Properties::create("B")};
    ClassicButton xButton{ClassicButton::Properties::create("X")};
    ClassicButton yButton{ClassicButton::Properties::create("Y")};

    juce::FlexBox layout;  // Main container
    juce::FlexBox rowLayout;  // For middle row (X + B)

    void setupCallbacks();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FaceButtons)
}; 