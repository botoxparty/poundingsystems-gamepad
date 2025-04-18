#include "DirectionalPad.h"

DirectionalPad::DirectionalPad()
{
    // Add child components
    addAndMakeVisible(upButton);
    addAndMakeVisible(downButton);
    addAndMakeVisible(leftButton);
    addAndMakeVisible(rightButton);

    setupCallbacks();
}

void DirectionalPad::setState(const State& newState)
{
    state = newState;

    // Update Up button
    upButton.setProperties({
        "Up",
        state.upCC,
        state.upPressed,
        state.isLearnMode
    });

    // Update Down button
    downButton.setProperties({
        "Down",
        state.downCC,
        state.downPressed,
        state.isLearnMode
    });

    // Update Left button
    leftButton.setProperties({
        "Left",
        state.leftCC,
        state.leftPressed,
        state.isLearnMode
    });

    // Update Right button
    rightButton.setProperties({
        "Right",
        state.rightCC,
        state.rightPressed,
        state.isLearnMode
    });
}

void DirectionalPad::resized()
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

    // Set up middle row items (Left + Right buttons)
    rowLayout.items.clear();
    rowLayout.items.add(juce::FlexItem(leftButton).withWidth(buttonSize).withHeight(buttonSize));
    rowLayout.items.add(juce::FlexItem().withWidth(buttonSize));  // Empty space in middle
    rowLayout.items.add(juce::FlexItem(rightButton).withWidth(buttonSize).withHeight(buttonSize));

    // Add items to main layout
    layout.items.clear();
    layout.items.add(juce::FlexItem(upButton).withWidth(buttonSize).withHeight(buttonSize));
    layout.items.add(juce::FlexItem(rowLayout).withWidth(bounds.getWidth()).withHeight(buttonSize));
    layout.items.add(juce::FlexItem(downButton).withWidth(buttonSize).withHeight(buttonSize));

    // Apply layouts
    layout.performLayout(bounds);
}

void DirectionalPad::paint(juce::Graphics& g)
{
    // Optional: Add any background styling if needed
}

void DirectionalPad::setupCallbacks()
{
    // Up button callbacks
    upButton.onClick = [this]() {
        if (onButtonClick)
            onButtonClick("Up");
    };
    
    upButton.onLearnClick = [this]() {
        if (onLearnClick)
            onLearnClick("Up");
    };

    // Down button callbacks
    downButton.onClick = [this]() {
        if (onButtonClick)
            onButtonClick("Down");
    };
    
    downButton.onLearnClick = [this]() {
        if (onLearnClick)
            onLearnClick("Down");
    };

    // Left button callbacks
    leftButton.onClick = [this]() {
        if (onButtonClick)
            onButtonClick("Left");
    };
    
    leftButton.onLearnClick = [this]() {
        if (onLearnClick)
            onLearnClick("Left");
    };

    // Right button callbacks
    rightButton.onClick = [this]() {
        if (onButtonClick)
            onButtonClick("Right");
    };
    
    rightButton.onLearnClick = [this]() {
        if (onLearnClick)
            onLearnClick("Right");
    };
} 