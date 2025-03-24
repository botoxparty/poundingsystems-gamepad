#include "FaceButtons.h"

FaceButtons::FaceButtons()
{
    // Add child components
    addAndMakeVisible(aButton);
    addAndMakeVisible(bButton);
    addAndMakeVisible(xButton);
    addAndMakeVisible(yButton);

    setupCallbacks();
}

void FaceButtons::setState(const State& newState)
{
    state = newState;

    // Update A button
    aButton.setProperties({
        "A",
        state.aCC,
        state.aPressed,
        state.isLearnMode
    });

    // Update B button
    bButton.setProperties({
        "B",
        state.bCC,
        state.bPressed,
        state.isLearnMode
    });

    // Update X button
    xButton.setProperties({
        "X",
        state.xCC,
        state.xPressed,
        state.isLearnMode
    });

    // Update Y button
    yButton.setProperties({
        "Y",
        state.yCC,
        state.yPressed,
        state.isLearnMode
    });
}

void FaceButtons::resized()
{
    auto bounds = getLocalBounds();
    float buttonSize = juce::jmin(bounds.getWidth() / 3.0f, bounds.getHeight() / 3.0f);

    // Configure main vertical layout
    layout.flexDirection = juce::FlexBox::Direction::column;
    layout.justifyContent = juce::FlexBox::JustifyContent::center;
    layout.alignItems = juce::FlexBox::AlignItems::center;

    // Configure middle row layout
    rowLayout.flexDirection = juce::FlexBox::Direction::row;
    rowLayout.justifyContent = juce::FlexBox::JustifyContent::center;
    rowLayout.alignItems = juce::FlexBox::AlignItems::center;

    // Set up middle row items (X + B buttons)
    rowLayout.items.clear();
    rowLayout.items.add(juce::FlexItem(xButton).withWidth(buttonSize).withHeight(buttonSize));
    rowLayout.items.add(juce::FlexItem().withWidth(buttonSize));  // Empty space in middle
    rowLayout.items.add(juce::FlexItem(bButton).withWidth(buttonSize).withHeight(buttonSize));

    // Add items to main layout
    layout.items.clear();
    layout.items.add(juce::FlexItem(yButton).withWidth(buttonSize).withHeight(buttonSize));
    layout.items.add(juce::FlexItem(rowLayout).withWidth(bounds.getWidth()).withHeight(buttonSize));
    layout.items.add(juce::FlexItem(aButton).withWidth(buttonSize).withHeight(buttonSize));

    // Apply layouts
    layout.performLayout(bounds);
}

void FaceButtons::paint(juce::Graphics& g)
{
    // Optional: Add any background styling if needed
}

void FaceButtons::setupCallbacks()
{
    // A button callbacks
    aButton.onClick = [this]() {
        if (onButtonClick)
            onButtonClick("A");
    };
    
    aButton.onLearnClick = [this]() {
        if (onLearnClick)
            onLearnClick("A");
    };

    // B button callbacks
    bButton.onClick = [this]() {
        if (onButtonClick)
            onButtonClick("B");
    };
    
    bButton.onLearnClick = [this]() {
        if (onLearnClick)
            onLearnClick("B");
    };

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
} 