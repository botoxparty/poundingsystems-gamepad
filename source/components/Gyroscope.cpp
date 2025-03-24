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
    updateButtonTexts();

    // Update X button
    xButton.setProperties({
        "X: " + juce::String(state.x, 2),
        state.xCC,
        false,
        state.isLearnMode,
        juce::Colours::red.withAlpha(0.7f),
        state.isLearnMode ? juce::Colours::white : juce::Colours::black
    });

    // Update Y button
    yButton.setProperties({
        "Y: " + juce::String(state.y, 2),
        state.yCC,
        false,
        state.isLearnMode,
        juce::Colours::green.withAlpha(0.7f),
        state.isLearnMode ? juce::Colours::white : juce::Colours::black
    });

    // Update Z button
    zButton.setProperties({
        "Z: " + juce::String(state.z, 2),
        state.zCC,
        false,
        state.isLearnMode,
        juce::Colours::blue.withAlpha(0.7f),
        state.isLearnMode ? juce::Colours::white : juce::Colours::black
    });
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
    xButton.onLearnClick = [this]() {
        if (onLearnClick)
            onLearnClick("X");
    };

    yButton.onLearnClick = [this]() {
        if (onLearnClick)
            onLearnClick("Y");
    };

    zButton.onLearnClick = [this]() {
        if (onLearnClick)
            onLearnClick("Z");
    };
}

void Gyroscope::updateButtonTexts()
{
    if (!state.enabled)
    {
        xButton.setProperties({"X: --", state.xCC, false, state.isLearnMode});
        yButton.setProperties({"Y: --", state.yCC, false, state.isLearnMode});
        zButton.setProperties({"Z: --", state.zCC, false, state.isLearnMode});
    }
} 