#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

/**
 * A trigger button component with progress bar style and MIDI learn capability
 * Used for L2/R2 triggers that have analog values
 */
class TriggerButton : public juce::Component {
public:
    struct Properties {
        juce::String text;
        int ccNumber;
        float value;
        bool isLearnMode;
        juce::Colour progressColor;
        juce::Colour backgroundColor;
        juce::Colour textColor;
    };

    explicit TriggerButton(const Properties& initialProps = Properties{
        "", // text
        -1, // ccNumber
        0.0f, // value
        false, // isLearnMode
        juce::Colour(0, 0, 255), // progressColor - Bright blue
        juce::Colour(220, 220, 220), // backgroundColor
        juce::Colours::black // textColor
    });

    ~TriggerButton() override = default;

    void setProperties(const Properties& newProps);
    const Properties& getProperties() const { return props; }

    // Event callbacks
    std::function<void()> onLearnClick;

    // Component overrides
    void paint(juce::Graphics& g) override;
    void mouseDown(const juce::MouseEvent& event) override;

private:
    Properties props;

    void drawClassicStyle(juce::Graphics& g);
    void drawLearnStyle(juce::Graphics& g);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TriggerButton)
}; 