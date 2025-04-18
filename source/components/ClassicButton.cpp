#include "ClassicButton.h"

ClassicButton::ClassicButton(const Properties& initialProps)
    : props(initialProps)
{
    setOpaque(true);
}

void ClassicButton::setProperties(const Properties& newProps)
{
    props = newProps;
    repaint();
}

void ClassicButton::paint(juce::Graphics& g)
{
    if (props.isLearnMode && props.ccNumber >= 0)
        drawLearnStyle(g);
    else
        drawClassicStyle(g);
}

void ClassicButton::drawClassicStyle(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    
    // Fill background
    g.setColour(props.backgroundColor);
    if (props.cornerRadius > 0.0f)
        g.fillRoundedRectangle(bounds, props.cornerRadius);
    else
        g.fillRect(bounds);

    // Draw 3D effect
    float borderWidth = 1.0f;
    g.setColour(props.isPressed || isMouseDown ? juce::Colours::darkgrey : juce::Colours::white);
    g.drawLine(bounds.getX(), bounds.getY(), bounds.getRight(), bounds.getY(), borderWidth);  // Top
    g.drawLine(bounds.getX(), bounds.getY(), bounds.getX(), bounds.getBottom(), borderWidth); // Left

    g.setColour(props.isPressed || isMouseDown ? juce::Colours::white : juce::Colours::darkgrey);
    g.drawLine(bounds.getRight(), bounds.getY(), bounds.getRight(), bounds.getBottom(), borderWidth); // Right
    g.drawLine(bounds.getX(), bounds.getBottom(), bounds.getRight(), bounds.getBottom(), borderWidth); // Bottom

    // Draw text
    if (props.text.isNotEmpty())
    {
        g.setColour(props.textColor);
        g.setFont(juce::Font("MS Sans Serif", 11.0f, juce::Font::plain));
        
        // If in learn mode and has CC number, show both text and CC
        if (props.isLearnMode && props.ccNumber >= 0)
        {
            auto textBounds = bounds.reduced(2);
            g.drawText(props.text, textBounds.removeFromTop(textBounds.getHeight() * 0.6f), 
                      juce::Justification::centred, false);
            g.setFont(juce::Font("MS Sans Serif", 9.0f, juce::Font::plain));
            g.drawText("CC" + juce::String(props.ccNumber), textBounds, 
                      juce::Justification::centred, false);
        }
        else
        {
            g.drawText(props.text, bounds, juce::Justification::centred, false);
        }
    }
}

void ClassicButton::drawLearnStyle(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    
    // Fill with blue background
    g.setColour(juce::Colours::blue.withAlpha(0.7f));
    if (props.cornerRadius > 0.0f)
        g.fillRoundedRectangle(bounds, props.cornerRadius);
    else
        g.fillRect(bounds);

    // Draw border
    g.setColour(juce::Colours::white);
    if (props.cornerRadius > 0.0f)
        g.drawRoundedRectangle(bounds, props.cornerRadius, 1.0f);
    else
        g.drawRect(bounds, 1.0f);

    // Draw text
    if (props.text.isNotEmpty())
    {
        g.setColour(juce::Colours::white);
        g.setFont(juce::Font("MS Sans Serif", 11.0f, juce::Font::plain));
        auto textBounds = bounds.reduced(2);
        
        g.drawText(props.text, textBounds.removeFromTop(textBounds.getHeight() * 0.6f), 
                  juce::Justification::centred, false);
        
        if (props.ccNumber >= 0)
        {
            g.setFont(juce::Font("MS Sans Serif", 9.0f, juce::Font::plain));
            g.drawText("CC" + juce::String(props.ccNumber), textBounds, 
                      juce::Justification::centred, false);
        }
    }
}

void ClassicButton::mouseDown(const juce::MouseEvent& event)
{
    isMouseDown = true;
    repaint();

    if (onPress)
        onPress();
}

void ClassicButton::mouseUp(const juce::MouseEvent& event)
{
    isMouseDown = false;
    repaint();
    
    if (onRelease)
        onRelease();
} 