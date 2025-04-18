#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class ClassicButton : public juce::Component {
public:
    struct Properties {
        juce::String text;
        int ccNumber = -1;
        bool isPressed = false;
        bool isLearnMode = false;
        juce::Colour backgroundColor = juce::Colour(220, 220, 220);
        juce::Colour textColor = juce::Colours::black;
        float cornerRadius = 0.0f;

        static Properties create(const juce::String& text) {
            Properties props;
            props.text = text;
            return props;
        }
    };

    explicit ClassicButton(const Properties& initialProps);
    ~ClassicButton() override = default;

    void setProperties(const Properties& newProps);
    const Properties& getProperties() const { return props; }

    // Event callbacks
    std::function<void()> onPress;   // Called when button is pressed down
    std::function<void()> onRelease; // Called when button is released

    // Component overrides
    void paint(juce::Graphics& g) override;
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseUp(const juce::MouseEvent& event) override;

private:
    Properties props;
    bool isMouseDown = false;

    void drawClassicStyle(juce::Graphics& g);
    void drawLearnStyle(juce::Graphics& g);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ClassicButton)
}; 