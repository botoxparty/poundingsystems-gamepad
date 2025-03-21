#include "GamepadComponent.h"

GamepadComponent::GamepadComponent(const GamepadManager::GamepadState& state)
    : gamepadState(state), cachedState(state)
{
    setupVisuals();
    startTimer(33); // ~30fps refresh rate
}

GamepadComponent::~GamepadComponent()
{
    stopTimer();
}

void GamepadComponent::setupVisuals()
{
    // Face buttons (A, B, X, Y)
    buttonVisuals.push_back({"A", {}, 0});
    buttonVisuals.push_back({"B", {}, 1});
    buttonVisuals.push_back({"X", {}, 2});
    buttonVisuals.push_back({"Y", {}, 3});
    
    // Menu buttons (Back, Guide, Start)
    buttonVisuals.push_back({"Back", {}, 4});
    buttonVisuals.push_back({"Guide", {}, 5});
    buttonVisuals.push_back({"Start", {}, 6});
    
    // Shoulder buttons
    buttonVisuals.push_back({"L1", {}, 9});
    buttonVisuals.push_back({"R1", {}, 10});
    
    // Stick buttons
    buttonVisuals.push_back({"L3", {}, 7});
    buttonVisuals.push_back({"R3", {}, 8});
    
    // D-Pad
    buttonVisuals.push_back({"Up", {}, 11});
    buttonVisuals.push_back({"Down", {}, 12});
    buttonVisuals.push_back({"Left", {}, 13});
    buttonVisuals.push_back({"Right", {}, 14});
    
    // Axes
    axisVisuals.push_back({"Left Stick", {}, 0, true});  // Represents both X and Y axes
    axisVisuals.push_back({"Right Stick", {}, 2, true}); // Represents both X and Y axes
    axisVisuals.push_back({"L2", {}, 4, false});
    axisVisuals.push_back({"R2", {}, 5, false});
}

void GamepadComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    
    // Draw background
    g.setColour(juce::Colours::darkgrey);
    g.fillRoundedRectangle(bounds, 10.0f);
    
    // Draw connection status
    g.setColour(juce::Colours::white);
    g.setFont(18.0f);
    
    juce::String statusText = cachedState.connected
        ? "Connected: " + cachedState.name
        : "Disconnected";
    
    g.drawText(statusText, bounds.removeFromTop(30.0f), juce::Justification::centred, false);
    
    if (!cachedState.connected)
        return;
    
    // Draw buttons
    float buttonSize = 30.0f;
    
    // Face buttons (right side)
    auto faceButtonsArea = bounds.removeFromRight(bounds.getWidth() * 0.25f).reduced(10.0f);
    
    // A, B, X, Y buttons in a diamond pattern
    float fbCenterX = faceButtonsArea.getCentreX();
    float fbCenterY = faceButtonsArea.getCentreY();
    
    // A button (bottom)
    buttonVisuals[0].bounds = juce::Rectangle<float>(fbCenterX - buttonSize/2, fbCenterY + buttonSize, buttonSize, buttonSize);
    
    // B button (right)
    buttonVisuals[1].bounds = juce::Rectangle<float>(fbCenterX + buttonSize, fbCenterY - buttonSize/2, buttonSize, buttonSize);
    
    // X button (left)
    buttonVisuals[2].bounds = juce::Rectangle<float>(fbCenterX - buttonSize*1.5f, fbCenterY - buttonSize/2, buttonSize, buttonSize);
    
    // Y button (top)
    buttonVisuals[3].bounds = juce::Rectangle<float>(fbCenterX - buttonSize/2, fbCenterY - buttonSize*1.5f, buttonSize, buttonSize);
    
    // D-Pad (left side)
    auto dpadArea = bounds.removeFromLeft(bounds.getWidth() * 0.33f).reduced(10.0f);
    float dpCenterX = dpadArea.getCentreX();
    float dpCenterY = dpadArea.getCentreY() + 50.0f; // Move D-pad a bit lower
    
    // D-pad Up
    buttonVisuals[11].bounds = juce::Rectangle<float>(dpCenterX - buttonSize/2, dpCenterY - buttonSize*1.5f, buttonSize, buttonSize);
    
    // D-pad Down
    buttonVisuals[12].bounds = juce::Rectangle<float>(dpCenterX - buttonSize/2, dpCenterY + buttonSize/2, buttonSize, buttonSize);
    
    // D-pad Left
    buttonVisuals[13].bounds = juce::Rectangle<float>(dpCenterX - buttonSize*1.5f, dpCenterY - buttonSize/2, buttonSize, buttonSize);
    
    // D-pad Right
    buttonVisuals[14].bounds = juce::Rectangle<float>(dpCenterX + buttonSize/2, dpCenterY - buttonSize/2, buttonSize, buttonSize);
    
    // Analog sticks
    float stickSize = 60.0f;
    float thumbstickBorderSize = 80.0f;
    
    // Left stick
    auto leftStickArea = dpadArea.withTrimmedBottom(dpadArea.getHeight() * 0.6f);
    axisVisuals[0].bounds = juce::Rectangle<float>(
        leftStickArea.getCentreX() - thumbstickBorderSize/2,
        leftStickArea.getCentreY() - thumbstickBorderSize/2,
        thumbstickBorderSize,
        thumbstickBorderSize
    );
    
    // Right stick
    auto rightStickArea = faceButtonsArea.withTrimmedBottom(faceButtonsArea.getHeight() * 0.6f);
    axisVisuals[1].bounds = juce::Rectangle<float>(
        rightStickArea.getCentreX() - thumbstickBorderSize/2,
        rightStickArea.getCentreY() - thumbstickBorderSize/2,
        thumbstickBorderSize,
        thumbstickBorderSize
    );
    
    // Set L3 and R3 button positions
    buttonVisuals[9].bounds = axisVisuals[0].bounds.reduced(thumbstickBorderSize/2 - buttonSize/2);
    buttonVisuals[10].bounds = axisVisuals[1].bounds.reduced(thumbstickBorderSize/2 - buttonSize/2);
    
    // Middle area (for triggers, shoulder buttons and menu buttons)
    auto middleArea = bounds;
    auto topArea = middleArea.removeFromTop(middleArea.getHeight() * 0.3f);
    auto centerArea = middleArea.removeFromTop(middleArea.getHeight() * 0.5f);
    
    // Shoulder buttons (L1, R1)
    float shoulderWidth = 60.0f;
    float shoulderHeight = 25.0f;
    buttonVisuals[7].bounds = juce::Rectangle<float>(
        topArea.getX() + 20.0f,
        topArea.getY() + 10.0f,
        shoulderWidth,
        shoulderHeight
    );
    
    buttonVisuals[8].bounds = juce::Rectangle<float>(
        topArea.getRight() - shoulderWidth - 20.0f,
        topArea.getY() + 10.0f,
        shoulderWidth,
        shoulderHeight
    );
    
    // Triggers (L2, R2)
    float triggerWidth = 60.0f;
    float triggerHeight = 20.0f;
    axisVisuals[2].bounds = juce::Rectangle<float>(
        topArea.getX() + 20.0f,
        topArea.getY() + shoulderHeight + 15.0f,
        triggerWidth,
        triggerHeight
    );
    
    axisVisuals[3].bounds = juce::Rectangle<float>(
        topArea.getRight() - triggerWidth - 20.0f,
        topArea.getY() + shoulderHeight + 15.0f,
        triggerWidth,
        triggerHeight
    );
    
    // Menu buttons (Back, Guide, Start)
    float menuButtonWidth = 40.0f;
    float menuButtonHeight = 20.0f;
    float menuButtonSpace = 10.0f;
    float totalMenuWidth = (menuButtonWidth * 3) + (menuButtonSpace * 2);
    float menuStartX = centerArea.getCentreX() - totalMenuWidth/2;
    
    // Back button
    buttonVisuals[4].bounds = juce::Rectangle<float>(
        menuStartX,
        centerArea.getCentreY() - menuButtonHeight/2,
        menuButtonWidth,
        menuButtonHeight
    );
    
    // Guide button
    buttonVisuals[5].bounds = juce::Rectangle<float>(
        menuStartX + menuButtonWidth + menuButtonSpace,
        centerArea.getCentreY() - menuButtonHeight/2,
        menuButtonWidth,
        menuButtonHeight
    );
    
    // Start button
    buttonVisuals[6].bounds = juce::Rectangle<float>(
        menuStartX + (menuButtonWidth + menuButtonSpace) * 2,
        centerArea.getCentreY() - menuButtonHeight/2,
        menuButtonWidth,
        menuButtonHeight
    );
    
    // Draw analog sticks
    for (const auto& axisVisual : axisVisuals)
    {
        if (axisVisual.isStick)
        {
            // Draw stick border
            g.setColour(juce::Colours::darkgrey.brighter(0.2f));
            g.drawEllipse(axisVisual.bounds, 2.0f);
            
            // Get stick position
            float x = 0.0f;
            float y = 0.0f;
            
            if (axisVisual.axisIndex == 0) // Left stick
            {
                x = cachedState.axes[0]; // X-axis
                y = cachedState.axes[1]; // Y-axis
            }
            else if (axisVisual.axisIndex == 2) // Right stick
            {
                x = cachedState.axes[2]; // X-axis
                y = cachedState.axes[3]; // Y-axis
            }
            
            // Calculate stick position within the border
            float centerX = axisVisual.bounds.getCentreX();
            float centerY = axisVisual.bounds.getCentreY();
            float radius = axisVisual.bounds.getWidth() * 0.35f;
            
            float stickX = centerX + (x * radius);
            float stickY = centerY + (y * radius);
            
            // Draw stick
            g.setColour(juce::Colours::lightgrey);
            g.fillEllipse(stickX - stickSize/2, stickY - stickSize/2, stickSize, stickSize);
            
            // Draw stick name
            g.setColour(juce::Colours::white);
            g.setFont(12.0f);
            g.drawText(axisVisual.name, 
                       axisVisual.bounds.getX(), 
                       axisVisual.bounds.getBottom() + 5.0f, 
                       axisVisual.bounds.getWidth(), 
                       20.0f, 
                       juce::Justification::centred, 
                       false);
        }
        else // Triggers
        {
            // Get trigger value (0.0 to 1.0)
            float value = (cachedState.axes[axisVisual.axisIndex] + 1.0f) * 0.5f;
            
            // Draw trigger background
            g.setColour(juce::Colours::darkgrey.brighter(0.1f));
            g.fillRoundedRectangle(axisVisual.bounds, 3.0f);
            
            // Draw trigger value
            juce::Rectangle<float> valueBounds = axisVisual.bounds.withWidth(axisVisual.bounds.getWidth() * value);
            
            g.setColour(juce::Colours::orange.withAlpha(0.8f));
            g.fillRoundedRectangle(valueBounds, 3.0f);
            
            // Draw trigger name
            g.setColour(juce::Colours::white);
            g.setFont(12.0f);
            g.drawText(axisVisual.name, 
                       axisVisual.bounds.getX(), 
                       axisVisual.bounds.getY() - 15.0f, 
                       axisVisual.bounds.getWidth(), 
                       15.0f, 
                       juce::Justification::centred, 
                       false);
        }
    }
    
    // Draw buttons
    for (const auto& buttonVisual : buttonVisuals)
    {
        bool isPressed = cachedState.buttons[buttonVisual.buttonIndex];
        
        g.setColour(isPressed ? juce::Colours::green.withAlpha(0.8f) : juce::Colours::darkgrey.brighter(0.1f));
        g.fillRoundedRectangle(buttonVisual.bounds, 5.0f);
        
        g.setColour(juce::Colours::white);
        g.setFont(12.0f);
        g.drawText(buttonVisual.name, 
                   buttonVisual.bounds, 
                   juce::Justification::centred, 
                   false);
        
        g.setColour(juce::Colours::lightgrey);
        g.drawRoundedRectangle(buttonVisual.bounds, 5.0f, 1.0f);
    }
}

void GamepadComponent::resized()
{
    
}

void GamepadComponent::updateState(const GamepadManager::GamepadState& newState)
{
    cachedState = newState;
    repaint();
}

void GamepadComponent::timerCallback()
{
    // Update the visualization with the current state
    updateState(gamepadState);
} 