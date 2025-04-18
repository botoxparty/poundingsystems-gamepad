#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "ClassicButton.h"
#include "TriggerButton.h"

/**
 * Component that contains the shoulder buttons (L1/R1) and triggers (L2/R2)
 */
class ShoulderSection : public juce::Component {
public:
    struct State {
        bool l1Pressed = false;
        bool r1Pressed = false;
        float l2Value = 0.0f;
        float r2Value = 0.0f;
        bool isLearnMode = false;
        int l1CC = -1;
        int r1CC = -1;
        int l2CC = -1;
        int r2CC = -1;
    };

    ShoulderSection();
    ~ShoulderSection() override = default;

    void setState(const State& newState);
    const State& getState() const { return state; }

    void setLearnMode(bool enabled) {
        state.isLearnMode = enabled;
        auto l1Props = l1Button.getProperties();
        l1Props.isLearnMode = enabled;
        l1Button.setProperties(l1Props);
        
        auto r1Props = r1Button.getProperties();
        r1Props.isLearnMode = enabled;
        r1Button.setProperties(r1Props);
        
        auto l2Props = l2Trigger.getProperties();
        l2Props.isLearnMode = enabled;
        l2Trigger.setProperties(l2Props);
        
        auto r2Props = r2Trigger.getProperties();
        r2Props.isLearnMode = enabled;
        r2Trigger.setProperties(r2Props);
        
        repaint();
    }

    // Event callbacks
    std::function<void(const juce::String&, float)> onButtonStateChanged;  // Called when button state changes (1.0f for press, 0.0f for release)

    // Component overrides
    void resized() override;

private:
    State state;

    ClassicButton l1Button{{"L1"}};
    ClassicButton r1Button{{"R1"}};
    TriggerButton l2Trigger{{
        "L2",  // text
        -1,    // ccNumber
        0.0f,  // value
        false, // isLearnMode
        juce::Colour(0, 0, 255), // progressColor - Bright blue
        juce::Colour(220, 220, 220), // backgroundColor
        juce::Colours::black // textColor
    }};
    TriggerButton r2Trigger{{
        "R2",  // text
        -1,    // ccNumber
        0.0f,  // value
        false, // isLearnMode
        juce::Colour(0, 0, 255), // progressColor - Bright blue
        juce::Colour(220, 220, 220), // backgroundColor
        juce::Colours::black // textColor
    }};

    juce::FlexBox layout;

    void setupCallbacks();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ShoulderSection)
}; 