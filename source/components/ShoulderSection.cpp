#include "ShoulderSection.h"

ShoulderSection::ShoulderSection()
{
    // Add child components
    addAndMakeVisible(l1Button);
    addAndMakeVisible(r1Button);
    addAndMakeVisible(l2Trigger);
    addAndMakeVisible(r2Trigger);

    setupCallbacks();
}

void ShoulderSection::setState(const State& newState)
{
    state = newState;

    // Update L1 button
    l1Button.setProperties({
        "L1",
        state.l1CC,
        state.l1Pressed,
        state.isLearnMode
    });

    // Update R1 button
    r1Button.setProperties({
        "R1",
        state.r1CC,
        state.r1Pressed,
        state.isLearnMode
    });

    // Update L2 trigger
    l2Trigger.setProperties({
        "L2",
        state.l2CC,
        state.l2Value,
        state.isLearnMode
    });

    // Update R2 trigger
    r2Trigger.setProperties({
        "R2",
        state.r2CC,
        state.r2Value,
        state.isLearnMode
    });
}

void ShoulderSection::resized()
{
    auto bounds = getLocalBounds();

    // Configure flex layout
    layout.flexDirection = juce::FlexBox::Direction::row;
    layout.justifyContent = juce::FlexBox::JustifyContent::spaceAround;
    layout.alignItems = juce::FlexBox::AlignItems::center;

    // Calculate component heights - use 80% of the available height
    float componentHeight = bounds.getHeight() * 0.8f;

    // Add items with equal flex and fixed height
    layout.items.clear();
    layout.items.add(juce::FlexItem(l1Button).withFlex(1.0f).withHeight(componentHeight));
    layout.items.add(juce::FlexItem(l2Trigger).withFlex(1.0f).withHeight(componentHeight));
    layout.items.add(juce::FlexItem(r1Button).withFlex(1.0f).withHeight(componentHeight));
    layout.items.add(juce::FlexItem(r2Trigger).withFlex(1.0f).withHeight(componentHeight));

    // Apply layout with some horizontal margin
    layout.performLayout(bounds.reduced(10, 0).toFloat());
}

void ShoulderSection::setupCallbacks()
{
    // L1 button callbacks
    l1Button.onClick = [this]() {
        if (onButtonClick)
            onButtonClick("L1");
    };
    
    l1Button.onLearnClick = [this]() {
        if (onLearnClick)
            onLearnClick("L1");
    };

    // R1 button callbacks
    r1Button.onClick = [this]() {
        if (onButtonClick)
            onButtonClick("R1");
    };
    
    r1Button.onLearnClick = [this]() {
        if (onLearnClick)
            onLearnClick("R1");
    };

    // L2 trigger callback
    l2Trigger.onLearnClick = [this]() {
        if (onLearnClick)
            onLearnClick("L2");
    };

    // R2 trigger callback
    r2Trigger.onLearnClick = [this]() {
        if (onLearnClick)
            onLearnClick("R2");
    };
} 