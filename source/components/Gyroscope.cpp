#include "Gyroscope.h"

Gyroscope::Gyroscope()
{
    // Add child components
    addAndMakeVisible(xButton);
    addAndMakeVisible(yButton);
    addAndMakeVisible(zButton);

    setupCallbacks();
}

void Gyroscope::setState(const State& newState)
{
    state = newState;

    // Update X button
    auto xProps = xButton.getProperties();
    xProps.text = state.enabled ? "X: " + juce::String(state.x, 2) : "X: --";
    xProps.ccNumber = state.xCC;
    xProps.isPressed = false;
    xProps.isLearnMode = state.isLearnMode;
    xProps.backgroundColor = juce::Colours::red.withAlpha(0.7f);
    xProps.textColor = state.isLearnMode ? juce::Colours::white : juce::Colours::black;
    xButton.setProperties(xProps);

    // Update Y button
    auto yProps = yButton.getProperties();
    yProps.text = state.enabled ? "Y: " + juce::String(state.y, 2) : "Y: --";
    yProps.ccNumber = state.yCC;
    yProps.isPressed = false;
    yProps.isLearnMode = state.isLearnMode;
    yProps.backgroundColor = juce::Colours::green.withAlpha(0.7f);
    yProps.textColor = state.isLearnMode ? juce::Colours::white : juce::Colours::black;
    yButton.setProperties(yProps);

    // Update Z button
    auto zProps = zButton.getProperties();
    zProps.text = state.enabled ? "Z: " + juce::String(state.z, 2) : "Z: --";
    zProps.ccNumber = state.zCC;
    zProps.isPressed = false;
    zProps.isLearnMode = state.isLearnMode;
    zProps.backgroundColor = juce::Colours::blue.withAlpha(0.7f);
    zProps.textColor = state.isLearnMode ? juce::Colours::white : juce::Colours::black;
    zButton.setProperties(zProps);
}

void Gyroscope::resized()
{
    auto bounds = getLocalBounds();

    // Configure flex layout
    layout.flexDirection = juce::FlexBox::Direction::column;
    layout.justifyContent = juce::FlexBox::JustifyContent::spaceAround;
    layout.alignItems = juce::FlexBox::AlignItems::stretch;

    // Add items with equal flex
    layout.items.clear();
    layout.items.add(juce::FlexItem(xButton).withFlex(1.0f).withMargin(juce::FlexItem::Margin(2.0f)));
    layout.items.add(juce::FlexItem(yButton).withFlex(1.0f).withMargin(juce::FlexItem::Margin(2.0f)));
    layout.items.add(juce::FlexItem(zButton).withFlex(1.0f).withMargin(juce::FlexItem::Margin(2.0f)));

    // Apply layout
    layout.performLayout(bounds);
}

void Gyroscope::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    
    // Draw background
    g.setColour(juce::Colours::darkgrey.brighter(0.1f));
    g.fillRoundedRectangle(bounds, 5.0f);
    
    // Draw border
    g.setColour(juce::Colours::lightgrey);
    g.drawRoundedRectangle(bounds, 5.0f, 1.0f);

    // Draw header
    g.setColour(juce::Colours::white);
    g.setFont(14.0f);
    auto headerArea = bounds.removeFromTop(25.0f);
    g.drawText("Gyroscope", headerArea, juce::Justification::centred, false);
}

void Gyroscope::setupCallbacks()
{
    // X button callbacks
    xButton.onClick = [this]() {
        if (onButtonClick)
            onButtonClick("X");
    };

    xButton.onLearnClick = [this]() {
        if (onLearnClick)
            onLearnClick("X");
    };

    // Y button callbacks
    yButton.onClick = [this]() {
        if (onButtonClick)
            onButtonClick("Y");
    };

    yButton.onLearnClick = [this]() {
        if (onLearnClick)
            onLearnClick("Y");
    };

    // Z button callbacks
    zButton.onClick = [this]() {
        if (onButtonClick)
            onButtonClick("Z");
    };

    zButton.onLearnClick = [this]() {
        if (onLearnClick)
            onLearnClick("Z");
    };
} 