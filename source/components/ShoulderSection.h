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

    // Event callbacks
    std::function<void(const juce::String&)> onLearnClick;  // Passes "L1", "R1", "L2", or "R2"
    std::function<void(const juce::String&)> onButtonClick; // For L1/R1 clicks

    // Component overrides
    void resized() override;

private:
    State state;

    ClassicButton l1Button{{"L1"}};
    ClassicButton r1Button{{"R1"}};
    TriggerButton l2Trigger{{"L2"}};
    TriggerButton r2Trigger{{"R2"}};

    juce::FlexBox layout;

    void setupCallbacks();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ShoulderSection)
}; 