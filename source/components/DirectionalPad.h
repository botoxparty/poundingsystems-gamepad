#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "ClassicButton.h"

/**
 * Component that displays a directional pad (D-pad) with MIDI learn capability
 */
class DirectionalPad : public juce::Component {
public:
    struct State {
        bool upPressed = false;
        bool downPressed = false;
        bool leftPressed = false;
        bool rightPressed = false;
        int upCC = -1;
        int downCC = -1;
        int leftCC = -1;
        int rightCC = -1;
        bool isLearnMode = false;
    };

    DirectionalPad();
    ~DirectionalPad() override = default;

    void setState(const State& newState);
    const State& getState() const { return state; }

    void setLearnMode(bool enabled) {
        state.isLearnMode = enabled;
        auto upProps = upButton.getProperties();
        upProps.isLearnMode = enabled;
        upButton.setProperties(upProps);
        
        auto downProps = downButton.getProperties();
        downProps.isLearnMode = enabled;
        downButton.setProperties(downProps);
        
        auto leftProps = leftButton.getProperties();
        leftProps.isLearnMode = enabled;
        leftButton.setProperties(leftProps);
        
        auto rightProps = rightButton.getProperties();
        rightProps.isLearnMode = enabled;
        rightButton.setProperties(rightProps);
        
        repaint();
    }

    // Event callbacks
    std::function<void(const juce::String&, float)> onButtonStateChanged;  // Called when button state changes (1.0f for press, 0.0f for release)

    // Component overrides
    void resized() override;
    void paint(juce::Graphics& g) override;

private:
    State state;

    ClassicButton upButton{{"Up"}};
    ClassicButton downButton{{"Down"}};
    ClassicButton leftButton{{"Left"}};
    ClassicButton rightButton{{"Right"}};

    juce::FlexBox layout;  // Main container
    juce::FlexBox rowLayout;  // For middle row (Left + Right)

    void setupCallbacks();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DirectionalPad)
}; 