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
    upButton.onPress = [this]() {
        if (onButtonStateChanged)
            onButtonStateChanged("Up", 1.0f);
    };
    
    upButton.onRelease = [this]() {
        if (onButtonStateChanged)
            onButtonStateChanged("Up", 0.0f);
    };

    // Down button callbacks
    downButton.onPress = [this]() {
        if (onButtonStateChanged)
            onButtonStateChanged("Down", 1.0f);
    };
    
    downButton.onRelease = [this]() {
        if (onButtonStateChanged)
            onButtonStateChanged("Down", 0.0f);
    };

    // Left button callbacks
    leftButton.onPress = [this]() {
        if (onButtonStateChanged)
            onButtonStateChanged("Left", 1.0f);
    };
    
    leftButton.onRelease = [this]() {
        if (onButtonStateChanged)
            onButtonStateChanged("Left", 0.0f);
    };

    // Right button callbacks
    rightButton.onPress = [this]() {
        if (onButtonStateChanged)
            onButtonStateChanged("Right", 1.0f);
    };
    
    rightButton.onRelease = [this]() {
        if (onButtonStateChanged)
            onButtonStateChanged("Right", 0.0f);
    };
} 