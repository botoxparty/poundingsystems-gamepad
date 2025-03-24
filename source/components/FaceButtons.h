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

    // Event callbacks
    std::function<void()> onAClick;
    std::function<void()> onBClick;
    std::function<void()> onXClick;
    std::function<void()> onYClick;
    std::function<void()> onALearnClick;
    std::function<void()> onBLearnClick;
    std::function<void()> onXLearnClick;
    std::function<void()> onYLearnClick;

    // Callbacks
    std::function<void(const juce::String&)> onButtonClick;
    std::function<void(const juce::String&)> onLearnClick;

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