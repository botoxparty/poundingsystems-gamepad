#include "AnalogStick.h"

AnalogStick::AnalogStick()
{
    // Add child components
    addAndMakeVisible(xButton);
    addAndMakeVisible(yButton);
    addAndMakeVisible(pressButton);

    setupCallbacks();
}

AnalogStick::AnalogStick(const juce::String& name, bool isStick)
{
    // Add child components
    addAndMakeVisible(xButton);
    addAndMakeVisible(yButton);
    addAndMakeVisible(pressButton);

    // Set initial state
    state.name = name;
    state.isStick = isStick;

    setupCallbacks();
}

void AnalogStick::setState(const State& newState)
{
    state = newState;

    // Update X axis button
    xButton.setProperties({
        "X",
        state.xCC,
        false,  // We don't show pressed state for axis
        state.isLearnMode
    });

    // Update Y axis button
    yButton.setProperties({
        "Y",
        state.yCC,
        false,  // We don't show pressed state for axis
        state.isLearnMode
    });

    // Update press button
    pressButton.setProperties({
        "Press",
        state.pressCC,
        state.isPressed,
        state.isLearnMode
    });

    updateStickPosition();
    repaint();
}

void AnalogStick::resized()
{
    auto bounds = getLocalBounds();
    auto buttonHeight = bounds.getHeight() / 8;

    // Configure button layout
    buttonLayout.flexDirection = juce::FlexBox::Direction::row;
    buttonLayout.justifyContent = juce::FlexBox::JustifyContent::spaceBetween;
    buttonLayout.alignItems = juce::FlexBox::AlignItems::center;

    // Add buttons to layout
    buttonLayout.items.clear();
    buttonLayout.items.add(juce::FlexItem(xButton).withHeight(buttonHeight).withWidth(bounds.getWidth() / 4));
    buttonLayout.items.add(juce::FlexItem(yButton).withHeight(buttonHeight).withWidth(bounds.getWidth() / 4));
    buttonLayout.items.add(juce::FlexItem(pressButton).withHeight(buttonHeight).withWidth(bounds.getWidth() / 4));

    // Configure main layout
    layout.flexDirection = juce::FlexBox::Direction::column;
    layout.justifyContent = juce::FlexBox::JustifyContent::spaceBetween;
    layout.alignItems = juce::FlexBox::AlignItems::center;

    // Calculate stick area
    auto stickArea = bounds.reduced(10);
    stickArea.removeFromBottom(buttonHeight + 10);  // Space for buttons
    stickBounds = stickArea.toFloat();

    // Add items to main layout
    layout.items.clear();
    layout.items.add(juce::FlexItem().withHeight(stickArea.getHeight()));
    layout.items.add(juce::FlexItem(buttonLayout).withWidth(bounds.getWidth()).withHeight(buttonHeight));

    // Apply layouts
    layout.performLayout(bounds);

    updateStickPosition();
}

void AnalogStick::paint(juce::Graphics& g)
{
    if (!state.isEnabled) return;

    // Draw stick area background
    g.setColour(juce::Colours::darkgrey);
    g.fillEllipse(stickBounds);

    // Draw deadzone
    g.setColour(juce::Colours::grey);
    auto centerX = stickBounds.getCentreX();
    auto centerY = stickBounds.getCentreY();
    g.drawEllipse(centerX - deadZoneRadius, centerY - deadZoneRadius,
                  deadZoneRadius * 2, deadZoneRadius * 2, 1.0f);

    drawLabels(g);
    drawStick(g);
}

void AnalogStick::setupCallbacks()
{
    // X axis button callbacks
    xButton.onClick = [this]() {
        if (onButtonClick)
            onButtonClick("X");
    };
    
    xButton.onLearnClick = [this]() {
        if (onLearnClick)
            onLearnClick("X");
    };

    // Y axis button callbacks
    yButton.onClick = [this]() {
        if (onButtonClick)
            onButtonClick("Y");
    };
    
    yButton.onLearnClick = [this]() {
        if (onLearnClick)
            onLearnClick("Y");
    };

    // Press button callbacks
    pressButton.onClick = [this]() {
        if (onButtonClick)
            onButtonClick("Press");
    };
    
    pressButton.onLearnClick = [this]() {
        if (onLearnClick)
            onLearnClick("Press");
    };
}

void AnalogStick::updateStickPosition()
{
    auto centerX = stickBounds.getCentreX();
    auto centerY = stickBounds.getCentreY();
    auto radius = (stickBounds.getWidth() - stickRadius * 2) / 2;

    // Calculate stick position based on x and y values (-1 to 1)
    float xPos = centerX + (state.xValue * radius);
    float yPos = centerY - (state.yValue * radius);  // Inverted Y for screen coordinates

    stickPosition.setXY(xPos, yPos);
}

void AnalogStick::drawStick(juce::Graphics& g)
{
    // Draw stick
    g.setColour(state.isPressed ? juce::Colours::red : juce::Colours::white);
    g.fillEllipse(stickPosition.x - stickRadius, stickPosition.y - stickRadius,
                  stickRadius * 2, stickRadius * 2);
}

void AnalogStick::drawLabels(juce::Graphics& g)
{
    g.setColour(juce::Colours::white);
    g.setFont(12.0f);

    // Draw axis labels
    auto centerX = stickBounds.getCentreX();
    auto centerY = stickBounds.getCentreY();
    auto radius = stickBounds.getWidth() / 2;

    // X axis labels
    g.drawText("-1", centerX - radius - 20, centerY - 10, 20, 20,
               juce::Justification::centred);
    g.drawText("+1", centerX + radius, centerY - 10, 20, 20,
               juce::Justification::centred);

    // Y axis labels
    g.drawText("+1", centerX - 10, centerY - radius - 20, 20, 20,
               juce::Justification::centred);
    g.drawText("-1", centerX - 10, centerY + radius, 20, 20,
               juce::Justification::centred);
} 