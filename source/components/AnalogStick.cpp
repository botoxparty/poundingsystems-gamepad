#include "AnalogStick.h"

AnalogStick::AnalogStick()
{
    // Add child components
    addAndMakeVisible(xButton);
    addAndMakeVisible(yButton);

    setupCallbacks();
}

AnalogStick::AnalogStick(const juce::String& name, bool isStick)
{
    // Add child components
    addAndMakeVisible(xButton);
    addAndMakeVisible(yButton);

    // Set initial state
    state.name = name;
    state.isStick = isStick;

    setupCallbacks();
}

void AnalogStick::setState(const State& newState)
{
    state = newState;

    // Update X axis button with current value
    xButton.setProperties({
        "X: " + juce::String(state.xValue, 2),
        state.xCC,
        false,  // We don't show pressed state for axis
        state.isLearnMode
    });

    // Update Y axis button with current value
    yButton.setProperties({
        "Y: " + juce::String(state.yValue, 2),
        state.yCC,
        false,  // We don't show pressed state for axis
        state.isLearnMode
    });

    updateStickPosition();
    repaint();
}

void AnalogStick::resized()
{
    auto bounds = getLocalBounds();
    auto buttonHeight = bounds.getHeight() / 4;

    // Configure button layout
    buttonLayout.flexDirection = juce::FlexBox::Direction::row;
    buttonLayout.justifyContent = juce::FlexBox::JustifyContent::spaceBetween;
    buttonLayout.alignItems = juce::FlexBox::AlignItems::center;

    // Add buttons to layout (removed press button)
    buttonLayout.items.clear();
    buttonLayout.items.add(juce::FlexItem(xButton).withHeight(buttonHeight).withWidth(bounds.getWidth() * 0.45f));
    buttonLayout.items.add(juce::FlexItem(yButton).withHeight(buttonHeight).withWidth(bounds.getWidth() * 0.45f));

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
    xButton.onPress = [this]() {
        if (state.isLearnMode) {
            if (onLearnClick)
                onLearnClick("X");
        } else {
            if (onButtonClick)
                onButtonClick("X");
        }
    };

    // Y axis button callbacks
    yButton.onPress = [this]() {
        if (state.isLearnMode) {
            if (onLearnClick)
                onLearnClick("Y");
        } else {
            if (onButtonClick)
                onButtonClick("Y");
        }
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
    // Draw stick background
    g.setColour(state.isLearnMode ? juce::Colour(0, 0, 139) : juce::Colours::darkgrey); // Dark blue in learn mode
    g.fillEllipse(stickPosition.x - stickRadius, stickPosition.y - stickRadius,
                  stickRadius * 2, stickRadius * 2);

    // Draw stick highlight/pressed state
    if (state.isLearnMode) {
        g.setColour(state.isPressed ? juce::Colours::red : juce::Colour(0, 0, 139));  // Dark blue when not pressed
    } else {
        g.setColour(state.isPressed ? juce::Colours::red : juce::Colours::white);
    }
    g.fillEllipse(stickPosition.x - (stickRadius * 0.8f), stickPosition.y - (stickRadius * 0.8f),
                  stickRadius * 1.6f, stickRadius * 1.6f);

    // Draw CC number in learn mode
    if (state.isLearnMode)
    {
        g.setColour(juce::Colours::white);
        g.setFont(12.0f);
        juce::String ccText = "CC" + juce::String(state.pressCC);
        g.drawText(ccText, 
                  stickPosition.x - stickRadius,
                  stickPosition.y - 6.0f,
                  stickRadius * 2,
                  12.0f,
                  juce::Justification::centred);
    }
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

void AnalogStick::mouseDown(const juce::MouseEvent& e)
{
    if (isPointOverStick(e.position.toFloat()))
    {
        state.isPressed = true;
        
        if (state.isLearnMode)
        {
            if (onLearnClick)
                onLearnClick("Press");
        }
        else
        {
            if (onButtonClick)
                onButtonClick("Press");
        }
        
        repaint();
    }
}

void AnalogStick::mouseUp(const juce::MouseEvent& e)
{
    if (state.isPressed)
    {
        state.isPressed = false;
        repaint();
    }
}

bool AnalogStick::isPointOverStick(juce::Point<float> point) const
{
    auto stickRect = juce::Rectangle<float>(
        stickPosition.x - stickRadius,
        stickPosition.y - stickRadius,
        stickRadius * 2,
        stickRadius * 2
    );
    return stickRect.contains(point);
} 