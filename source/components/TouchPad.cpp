#include "TouchPad.h"

TouchPad::TouchPad()
{
    // Add child components
    addAndMakeVisible(xButton);
    addAndMakeVisible(yButton);
    addAndMakeVisible(pressureButton);
    addAndMakeVisible(buttonPressButton);

    setupCallbacks();
}

void TouchPad::setState(const State& newState)
{
    state = newState;
    
    // Set touched state based on pressure
    state.touched = state.pressure > 0.0f;

    // Update X axis button with value
    xButton.setProperties({
        "X: " + juce::String(state.xValue, 2),
        state.xCC,
        false,  // We don't show pressed state for axis
        state.isLearnMode
    });

    // Update Y axis button with value
    yButton.setProperties({
        "Y: " + juce::String(state.yValue, 2),
        state.yCC,
        false,  // We don't show pressed state for axis
        state.isLearnMode
    });

    // Update pressure button
    pressureButton.setProperties({
        "P: " + juce::String(state.pressure, 2),
        state.pressureCC,
        false,  // We don't show pressed state for pressure
        state.isLearnMode
    });

    // Update button press button
    buttonPressButton.setProperties({
        "Button: " + juce::String(state.isPressed ? "On" : "Off"),
        state.buttonCC,
        state.isPressed,  // Show pressed state for button
        state.isLearnMode
    });

    updateTouchPosition();
    repaint();
}

void TouchPad::resized()
{
    auto bounds = getLocalBounds();
    auto buttonHeight = bounds.getHeight() / 6;

    // Configure button layout
    buttonLayout.flexDirection = juce::FlexBox::Direction::row;
    buttonLayout.justifyContent = juce::FlexBox::JustifyContent::center;
    buttonLayout.alignItems = juce::FlexBox::AlignItems::center;

    // Add buttons to layout with no margins
    buttonLayout.items.clear();
    auto buttonWidth = bounds.getWidth() / 4;
    buttonLayout.items.add(juce::FlexItem(xButton).withHeight(buttonHeight).withWidth(buttonWidth).withMargin(0));
    buttonLayout.items.add(juce::FlexItem(yButton).withHeight(buttonHeight).withWidth(buttonWidth).withMargin(0));
    buttonLayout.items.add(juce::FlexItem(pressureButton).withHeight(buttonHeight).withWidth(buttonWidth).withMargin(0));
    buttonLayout.items.add(juce::FlexItem(buttonPressButton).withHeight(buttonHeight).withWidth(buttonWidth).withMargin(0));

    // Configure main layout
    layout.flexDirection = juce::FlexBox::Direction::column;
    layout.justifyContent = juce::FlexBox::JustifyContent::flexStart;
    layout.alignItems = juce::FlexBox::AlignItems::stretch;

    // Calculate touch area - only reduce sides, not top/bottom
    auto area = bounds;
    area.removeFromBottom(buttonHeight);  // Remove exact space for buttons
    touchArea = area.toFloat();

    // Add items to main layout with no margins
    layout.items.clear();
    layout.items.add(juce::FlexItem().withHeight(area.getHeight()).withMargin(0));
    layout.items.add(juce::FlexItem(buttonLayout).withWidth(bounds.getWidth()).withHeight(buttonHeight).withMargin(0));

    // Apply layouts
    layout.performLayout(bounds);

    updateTouchPosition();
}

void TouchPad::paint(juce::Graphics& g)
{
    if (!state.isEnabled) return;

    drawTouchArea(g);
    drawLabels(g);
}

void TouchPad::setupCallbacks()
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

    // Pressure button callbacks
    pressureButton.onPress = [this]() {
        if (state.isLearnMode) {
            if (onLearnClick)
                onLearnClick("Pressure");
        } else {
            if (onButtonClick)
                onButtonClick("Pressure");
        }
    };

    // Button press button callbacks
    buttonPressButton.onPress = [this]() {
        if (state.isLearnMode) {
            if (onLearnClick)
                onLearnClick("Button");
        } else {
            if (onButtonClick)
                onButtonClick("Button");
        }
    };
}

void TouchPad::updateTouchPosition()
{
    float x = touchArea.getX() + (state.xValue * touchArea.getWidth());
    float y = touchArea.getY() + (state.yValue * touchArea.getHeight());
    touchPosition.setXY(x, y);
}

void TouchPad::drawTouchArea(juce::Graphics& g)
{
    // Draw touch area background
    g.setColour(juce::Colours::darkgrey);
    g.fillRect(touchArea);

    // Draw grid lines
    g.setColour(juce::Colours::grey);
    for (int i = 1; i < 4; ++i)
    {
        float x = touchArea.getX() + (i * touchArea.getWidth() / 4);
        float y = touchArea.getY() + (i * touchArea.getHeight() / 4);
        
        g.drawVerticalLine(static_cast<int>(x), touchArea.getY(), touchArea.getBottom());
        g.drawHorizontalLine(static_cast<int>(y), touchArea.getX(), touchArea.getRight());
    }

    // Draw touch point when touchpad is being touched
    if (state.touched)
    {
        g.setColour(juce::Colours::red);
        g.fillEllipse(touchPosition.x - touchRadius, touchPosition.y - touchRadius,
                      touchRadius * 2, touchRadius * 2);
    }
}

void TouchPad::drawLabels(juce::Graphics& g)
{
    // Labels are now shown in the buttons instead
} 