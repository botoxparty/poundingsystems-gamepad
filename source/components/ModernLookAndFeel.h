#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

/**
 * Custom look and feel for the application
 * Provides a consistent modern style for all buttons and components
 */
class ModernLookAndFeel : public juce::LookAndFeel_V4
{
public:
    ModernLookAndFeel();
    ~ModernLookAndFeel() override = default;

    // Button drawing methods
    void drawButtonBackground(juce::Graphics& g, juce::Button& button,
                             const juce::Colour& backgroundColour,
                             bool shouldDrawButtonAsHighlighted,
                             bool shouldDrawButtonAsDown) override;
    
    void drawButtonText(juce::Graphics& g, juce::TextButton& button,
                        bool shouldDrawButtonAsHighlighted,
                        bool shouldDrawButtonAsDown) override;
    
    int getTextButtonWidthToFitText(juce::TextButton& button, int maxWidth) override;
    
    // Custom colors
    juce::Colour getButtonBackgroundColour() const { return buttonBackgroundColour; }
    juce::Colour getButtonTextColour() const { return buttonTextColour; }
    juce::Colour getButtonHighlightColour() const { return buttonHighlightColour; }
    juce::Colour getButtonPressedColour() const { return buttonPressedColour; }
    
    // Custom dimensions
    int getButtonCornerRadius() const { return buttonCornerRadius; }
    int getButtonPadding() const { return buttonPadding; }
    
private:
    // Custom colors
    juce::Colour buttonBackgroundColour = juce::Colour(220, 220, 220);
    juce::Colour buttonTextColour = juce::Colours::black;
    juce::Colour buttonHighlightColour = juce::Colour(200, 200, 200);
    juce::Colour buttonPressedColour = juce::Colour(180, 180, 180);
    
    // Custom dimensions
    int buttonCornerRadius = 4;
    int buttonPadding = 8;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModernLookAndFeel)
}; 