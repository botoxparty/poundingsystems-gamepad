#include "TouchPad.h"

TouchPad::TouchPad()
{
    // Add child components
    addAndMakeVisible(xButton);
    addAndMakeVisible(yButton);
    addAndMakeVisible(pressureButton);

    setupCallbacks();
}

void TouchPad::setState(const State& newState)
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

    // Update pressure button
    pressureButton.setProperties({
        "Pressure",
        state.pressureCC,
        false,  // We don't show pressed state for pressure
        state.isLearnMode
    });

    updateTouchPosition();
    repaint();
}

void TouchPad::resized()
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
    buttonLayout.items.add(juce::FlexItem(pressureButton).withHeight(buttonHeight).withWidth(bounds.getWidth() / 4));

    // Configure main layout
    layout.flexDirection = juce::FlexBox::Direction::column;
    layout.justifyContent = juce::FlexBox::JustifyContent::spaceBetween;
    layout.alignItems = juce::FlexBox::AlignItems::center;

    // Calculate touch area
    auto area = bounds.reduced(10);
    area.removeFromBottom(buttonHeight + 10);  // Space for buttons
    area.removeFromBottom(pressureBarHeight + 5);  // Space for pressure bar
    touchArea = area.toFloat();

    // Add items to main layout
    layout.items.clear();
    layout.items.add(juce::FlexItem().withHeight(area.getHeight()));
    layout.items.add(juce::FlexItem().withHeight(pressureBarHeight));
    layout.items.add(juce::FlexItem(buttonLayout).withWidth(bounds.getWidth()).withHeight(buttonHeight));

    // Apply layouts
    layout.performLayout(bounds);

    updateTouchPosition();
}

void TouchPad::paint(juce::Graphics& g)
{
    if (!state.isEnabled) return;

    drawTouchArea(g);
    drawPressureBar(g);
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

void TouchPad::drawPressureBar(juce::Graphics& g)
{
    auto bounds = getLocalBounds().reduced(10);
    auto barBounds = bounds.removeFromBottom(pressureBarHeight + 5)
                          .removeFromTop(pressureBarHeight)
                          .toFloat();

    // Draw background
    g.setColour(juce::Colours::darkgrey);
    g.fillRect(barBounds);

    // Draw pressure level
    g.setColour(juce::Colours::red);
    auto pressureWidth = barBounds.getWidth() * state.pressure;
    g.fillRect(barBounds.withWidth(pressureWidth));

    // Draw border
    g.setColour(juce::Colours::grey);
    g.drawRect(barBounds, 1.0f);
}

void TouchPad::drawLabels(juce::Graphics& g)
{
    g.setColour(juce::Colours::white);
    g.setFont(12.0f);

    // Draw coordinate values
    juce::String coords = juce::String(state.xValue, 2) + ", " + 
                         juce::String(state.yValue, 2);
    g.drawText(coords, touchArea, juce::Justification::topLeft);

    // Draw pressure value
    juce::String pressure = "Pressure: " + juce::String(state.pressure, 2);
    auto bounds = getLocalBounds().reduced(10);
    auto textBounds = bounds.removeFromBottom(pressureBarHeight + 5)
                           .removeFromTop(pressureBarHeight);
    g.drawText(pressure, textBounds, juce::Justification::centredRight);
} 