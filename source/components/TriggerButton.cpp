#include "TriggerButton.h"

TriggerButton::TriggerButton(const Properties& initialProps)
    : props(initialProps)
{
    setOpaque(true);
}

void TriggerButton::setProperties(const Properties& newProps)
{
    juce::Logger::writeToLog("TriggerButton '" + newProps.text + "' value: " + juce::String(newProps.value));
    props = newProps;
    repaint();
}

void TriggerButton::paint(juce::Graphics& g)
{
    if (props.isLearnMode && props.ccNumber >= 0)
        drawLearnStyle(g);
    else
        drawClassicStyle(g);
}

void TriggerButton::drawClassicStyle(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    
    // Log component dimensions and value
    juce::Logger::writeToLog("TriggerButton '" + props.text + "' - Bounds: " + 
                            juce::String(bounds.getWidth()) + "x" + juce::String(bounds.getHeight()) + 
                            ", Value: " + juce::String(props.value));
    
    // Draw background with inset effect
    g.setColour(props.backgroundColor);
    g.fillRect(bounds);
    
    // Draw 3D inset effect
    float borderWidth = 1.0f;
    g.setColour(juce::Colours::darkgrey);
    g.drawLine(bounds.getX(), bounds.getY(), bounds.getRight(), bounds.getY(), borderWidth);  // Top
    g.drawLine(bounds.getX(), bounds.getY(), bounds.getX(), bounds.getBottom(), borderWidth); // Left
    
    g.setColour(juce::Colours::white);
    g.drawLine(bounds.getRight(), bounds.getY(), bounds.getRight(), bounds.getBottom(), borderWidth); // Right
    g.drawLine(bounds.getX(), bounds.getBottom(), bounds.getRight(), bounds.getBottom(), borderWidth); // Bottom

    // Draw progress bar
    auto progressBounds = bounds.reduced(2);
    auto valueBounds = progressBounds.withWidth(progressBounds.getWidth() * props.value);
    
    g.setColour(props.progressColor);
    g.fillRect(valueBounds);

    // Draw text
    if (props.text.isNotEmpty())
    {
        g.setColour(props.textColor);
        g.setFont(juce::Font("MS Sans Serif", 11.0f, juce::Font::plain));
        auto textBounds = bounds.reduced(2);
        
        // Draw main text
        juce::String labelText = props.text;
        labelText += ": " + juce::String(static_cast<int>(props.value * 127));
        g.drawText(labelText, textBounds, juce::Justification::centred, false);
        
        // Draw CC number if in learn mode
        if (props.isLearnMode && props.ccNumber >= 0)
        {
            g.setFont(juce::Font("MS Sans Serif", 9.0f, juce::Font::plain));
            g.drawText("CC" + juce::String(props.ccNumber), 
                      bounds.getX(), 
                      bounds.getBottom() + 2.0f, 
                      bounds.getWidth(), 
                      12.0f, 
                      juce::Justification::centred, 
                      false);
        }
    }
}

void TriggerButton::drawLearnStyle(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    
    // Fill with blue background
    g.setColour(juce::Colours::blue.withAlpha(0.7f));
    g.fillRect(bounds);

    // Draw border
    g.setColour(juce::Colours::white);
    g.drawRect(bounds, 1.0f);

    // Draw text
    if (props.text.isNotEmpty())
    {
        g.setColour(juce::Colours::white);
        g.setFont(juce::Font("MS Sans Serif", 11.0f, juce::Font::plain));
        auto textBounds = bounds.reduced(2);
        
        // Draw main text
        juce::String labelText = props.text;
        labelText += ": " + juce::String(static_cast<int>(props.value * 127));
        g.drawText(labelText, textBounds.removeFromTop(textBounds.getHeight() * 0.6f), 
                  juce::Justification::centred, false);
        
        // Draw CC number if assigned
        if (props.ccNumber >= 0)
        {
            g.setFont(juce::Font("MS Sans Serif", 9.0f, juce::Font::plain));
            g.drawText("CC" + juce::String(props.ccNumber), textBounds, 
                      juce::Justification::centred, false);
        }
    }
}

void TriggerButton::mouseDown(const juce::MouseEvent& event)
{
    if (props.isLearnMode && onLearnClick)
        onLearnClick();
} 