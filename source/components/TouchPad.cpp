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

    // Pressure button callbacks
    pressureButton.onClick = [this]() {
        if (onButtonClick)
            onButtonClick("Pressure");
    };
    
    pressureButton.onLearnClick = [this]() {
        if (onLearnClick)
            onLearnClick("Pressure");
    };

    // Button press button callbacks
    buttonPressButton.onClick = [this]() {
        if (onButtonClick)
            onButtonClick("Button");
    };
    
    buttonPressButton.onLearnClick = [this]() {
        if (onLearnClick)
            onLearnClick("Button");
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

    // Draw touch point
    if (state.isPressed)
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

void TouchPad::mouseDown(const juce::MouseEvent& e)
{
    if (!state.isEnabled) return;

    if (touchArea.contains(e.position)) {
        state.isPressed = true;
        updateTouchValuesFromMouse(e);
        repaint();
    }
}

void TouchPad::mouseDrag(const juce::MouseEvent& e)
{
    if (!state.isEnabled || !state.isPressed) return;

    if (touchArea.contains(e.position)) {
        updateTouchValuesFromMouse(e);
        repaint();
    }
}

void TouchPad::mouseUp(const juce::MouseEvent& e)
{
    if (!state.isEnabled) return;

    state.isPressed = false;
    
    // Send button release if not in learn mode
    if (!state.isLearnMode && onButtonValueChange) {
        onButtonValueChange(0.0f);  // Button released
    }
    
    repaint();
}

void TouchPad::updateTouchValuesFromMouse(const juce::MouseEvent& e)
{
    // Calculate normalized values (0.0 to 1.0)
    float x = (e.position.x - touchArea.getX()) / touchArea.getWidth();
    float y = (e.position.y - touchArea.getY()) / touchArea.getHeight();
    
    // Clamp values between 0 and 1
    x = juce::jlimit(0.0f, 1.0f, x);
    y = juce::jlimit(0.0f, 1.0f, y);
    
    // Get pressure from the mouse event if available, otherwise use 1.0
    float pressure = e.pressure > 0.0f ? e.pressure : 1.0f;
    
    // Send MIDI if not in learn mode
    if (!state.isLearnMode) {
        if (onXValueChange) onXValueChange(x);
        if (onYValueChange) onYValueChange(y);
        if (onPressureValueChange) onPressureValueChange(pressure);
        if (onButtonValueChange) onButtonValueChange(1.0f);  // Button pressed
    }
    
    // Update state
    state.xValue = x;
    state.yValue = y;
    state.pressure = pressure;
    state.isPressed = true;
    
    // Update the visual position
    updateTouchPosition();
} 