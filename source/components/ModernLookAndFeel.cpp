#include "ModernLookAndFeel.h"

ModernLookAndFeel::ModernLookAndFeel()
{
    // Initialize with default settings
}

void ModernLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button,
                                            const juce::Colour& backgroundColour,
                                            bool shouldDrawButtonAsHighlighted,
                                            bool shouldDrawButtonAsDown)
{
    auto bounds = button.getLocalBounds().toFloat();
    
    // Determine the color based on button state
    juce::Colour color;
    if (shouldDrawButtonAsDown)
        color = buttonPressedColour;
    else if (shouldDrawButtonAsHighlighted)
        color = buttonHighlightColour;
    else
        color = buttonBackgroundColour;
    
    // Draw the button background with rounded corners
    g.setColour(color);
    g.fillRoundedRectangle(bounds, buttonCornerRadius);
    
    // Draw a subtle border
    g.setColour(juce::Colours::darkgrey.withAlpha(0.5f));
    g.drawRoundedRectangle(bounds, buttonCornerRadius, 1.0f);
}

void ModernLookAndFeel::drawButtonText(juce::Graphics& g, juce::TextButton& button,
                                      bool shouldDrawButtonAsHighlighted,
                                      bool shouldDrawButtonAsDown)
{
    auto bounds = button.getLocalBounds();
    
    // Set the text color
    g.setColour(buttonTextColour);
    
    // Set the font
    g.setFont(juce::Font(14.0f, juce::Font::plain));
    
    // Draw the text
    g.drawText(button.getButtonText(), bounds, juce::Justification::centred, false);
}

int ModernLookAndFeel::getTextButtonWidthToFitText(juce::TextButton& button, int maxWidth)
{
    // Get the font
    juce::Font font(14.0f, juce::Font::plain);
    
    // Calculate the width needed for the text
    int textWidth = font.getStringWidth(button.getButtonText());
    
    // Add padding
    int totalWidth = textWidth + (buttonPadding * 2);
    
    // Ensure it doesn't exceed the maximum width
    return juce::jmin(totalWidth, maxWidth);
} 