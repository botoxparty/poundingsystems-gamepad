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
    
    // Draw classic Windows style background with beveled edge
    g.setColour(juce::Colour(192, 192, 192));  // System gray
    g.fillAll();
    
    // Draw inset panel effect
    g.setColour(juce::Colours::darkgrey);
    g.drawLine(0, 0, getWidth() - 1, 0, 1.0f);  // Top
    g.drawLine(0, 0, 0, getHeight() - 1, 1.0f);  // Left
    
    g.setColour(juce::Colours::white);
    g.drawLine(getWidth() - 1, 0, getWidth() - 1, getHeight() - 1, 1.0f);  // Right
    g.drawLine(0, getHeight() - 1, getWidth() - 1, getHeight() - 1, 1.0f);  // Bottom
    
    // Draw connection status
    g.setColour(juce::Colours::black);
    g.setFont(juce::Font("MS Sans Serif", 12.0f, juce::Font::plain));
    
    juce::String statusText = cachedState.connected
        ? "Connected: " + cachedState.name
        : "Disconnected";
    
    // Draw status text in a classic Windows style group box
    auto statusBounds = bounds.removeFromTop(30.0f).reduced(5);
    drawClassicGroupBox(g, statusBounds, statusText);
    
    if (!cachedState.connected)
        return;
    
    // Draw buttons with classic Windows style
    float buttonSize = 30.0f;
    
    // Face buttons (right side)
    auto faceButtonsArea = bounds.removeFromRight(bounds.getWidth() * 0.25f).reduced(10.0f);
    auto faceButtonTopArea = faceButtonsArea.removeFromTop(faceButtonsArea.getHeight() * 0.4f);
    auto faceButtonBottomArea = faceButtonsArea;
    
    // D-Pad (left side)
    auto dpadArea = bounds.removeFromLeft(bounds.getWidth() * 0.33f).reduced(10.0f);
    auto dpadTopArea = dpadArea.removeFromTop(dpadArea.getHeight() * 0.4f);
    auto dpadBottomArea = dpadArea;
    
    // A, B, X, Y buttons in a diamond pattern (exactly matching D-pad pattern)
    float fbCenterX = faceButtonTopArea.getCentreX();
    float fbCenterY = faceButtonTopArea.getCentreY();
    
    // Y button (top) - matches D-pad Up
    buttonVisuals[3].bounds = juce::Rectangle<float>(fbCenterX - buttonSize/2, fbCenterY - buttonSize*1.5f, buttonSize, buttonSize);
    
    // A button (bottom) - matches D-pad Down
    buttonVisuals[0].bounds = juce::Rectangle<float>(fbCenterX - buttonSize/2, fbCenterY + buttonSize/2, buttonSize, buttonSize);
    
    // X button (left) - matches D-pad Left
    buttonVisuals[2].bounds = juce::Rectangle<float>(fbCenterX - buttonSize*1.5f, fbCenterY - buttonSize/2, buttonSize, buttonSize);
    
    // B button (right) - matches D-pad Right
    buttonVisuals[1].bounds = juce::Rectangle<float>(fbCenterX + buttonSize/2, fbCenterY - buttonSize/2, buttonSize, buttonSize);
    
    // D-pad positioning
    float dpCenterX = dpadTopArea.getCentreX();
    float dpCenterY = dpadTopArea.getCentreY();
    
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
    
    // Left stick (now below D-pad, with space for label)
    float labelSpacing = 25.0f;  // Space for label below stick
    axisVisuals[0].bounds = juce::Rectangle<float>(
        dpadBottomArea.getCentreX() - thumbstickBorderSize/2,
        dpadBottomArea.getCentreY() - thumbstickBorderSize/2 - labelSpacing/2,  // Move up to make room for label
        thumbstickBorderSize,
        thumbstickBorderSize
    );
    
    // Right stick (now below face buttons, with space for label)
    axisVisuals[1].bounds = juce::Rectangle<float>(
        faceButtonBottomArea.getCentreX() - thumbstickBorderSize/2,
        faceButtonBottomArea.getCentreY() - thumbstickBorderSize/2 - labelSpacing/2,  // Move up to make room for label
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
    
    // Add touchpad in a higher position of the center area
    auto touchpadArea = centerArea.removeFromTop(centerArea.getHeight() * 0.5f); // Adjusted for higher placement
    juce::Rectangle<float> touchpadBounds(
        touchpadArea.getCentreX() - 100.0f,
        touchpadArea.getCentreY() - 15.0f, // Move up by subtracting 15 pixels instead of adding
        200.0f,    // width
        60.0f      // height
    );
    
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
        topArea.getY() + shoulderHeight + 30.0f,
        triggerWidth,
        triggerHeight
    );
    
    axisVisuals[3].bounds = juce::Rectangle<float>(
        topArea.getRight() - triggerWidth - 20.0f,
        topArea.getY() + shoulderHeight + 30.0f,
        triggerWidth,
        triggerHeight
    );
    
    // Menu buttons (Back, Guide, Start) - moved lower
    float menuButtonWidth = 40.0f;
    float menuButtonHeight = 20.0f;
    float menuButtonSpace = 10.0f;
    float totalMenuWidth = (menuButtonWidth * 3) + (menuButtonSpace * 2);
    float menuStartX = centerArea.getCentreX() - totalMenuWidth/2;
    
    // Position menu buttons with a safe margin from the touchpad
    // but still within the visible area of the component
    float menuY = touchpadArea.getBottom() + 30.0f;
    
    // Back button
    buttonVisuals[4].bounds = juce::Rectangle<float>(
        menuStartX,
        menuY,
        menuButtonWidth,
        menuButtonHeight
    );
    
    // Guide button
    buttonVisuals[5].bounds = juce::Rectangle<float>(
        menuStartX + menuButtonWidth + menuButtonSpace,
        menuY,
        menuButtonWidth,
        menuButtonHeight
    );
    
    // Start button
    buttonVisuals[6].bounds = juce::Rectangle<float>(
        menuStartX + (menuButtonWidth + menuButtonSpace) * 2,
        menuY,
        menuButtonWidth,
        menuButtonHeight
    );
    
    // Draw analog sticks with classic Windows style
    for (const auto& axisVisual : axisVisuals)
    {
        if (axisVisual.isStick)
        {
            // Draw stick border (inset effect)
            drawClassicInsetPanel(g, axisVisual.bounds);
            
            // Get stick position
            float x = 0.0f;
            float y = 0.0f;
            
            if (axisVisual.axisIndex == 0) // Left stick
            {
                x = cachedState.axes[0];
                y = cachedState.axes[1];
            }
            else if (axisVisual.axisIndex == 2) // Right stick
            {
                x = cachedState.axes[2];
                y = cachedState.axes[3];
            }
            
            // Calculate stick position
            float centerX = axisVisual.bounds.getCentreX();
            float centerY = axisVisual.bounds.getCentreY();
            float radius = axisVisual.bounds.getWidth() * 0.35f;
            
            float stickX = centerX + (x * radius);
            float stickY = centerY + (y * radius);
            
            // Draw stick with classic button style
            auto stickBounds = juce::Rectangle<float>(stickX - buttonSize/2, stickY - buttonSize/2, buttonSize, buttonSize);
            drawClassicButton(g, stickBounds, true);
            
            // Draw stick name below the stick
            g.setColour(juce::Colours::black);
            g.setFont(juce::Font("MS Sans Serif", 11.0f, juce::Font::plain));
            g.drawText(axisVisual.name, 
                      axisVisual.bounds.getX(), 
                      axisVisual.bounds.getBottom() + 5.0f, 
                      axisVisual.bounds.getWidth(), 
                      15.0f, 
                      juce::Justification::centred, 
                      false);
        }
        else // Triggers
        {
            // Draw trigger background with inset effect
            drawClassicInsetPanel(g, axisVisual.bounds);
            
            // Get trigger value
            float value = (cachedState.axes[axisVisual.axisIndex] + 1.0f) * 0.5f;
            
            // Draw trigger value with classic progress bar style
            auto valueBounds = axisVisual.bounds.withWidth(axisVisual.bounds.getWidth() * value);
            g.setColour(juce::Colour(0, 0, 128));  // Classic Windows blue
            g.fillRect(valueBounds.reduced(2));
            
            // Draw trigger name
            g.setColour(juce::Colours::black);
            g.setFont(juce::Font("MS Sans Serif", 11.0f, juce::Font::plain));
            g.drawText(axisVisual.name, 
                      axisVisual.bounds.getX(), 
                      axisVisual.bounds.getY() - 15.0f, 
                      axisVisual.bounds.getWidth(), 
                      15.0f, 
                      juce::Justification::centred, 
                      false);
        }
    }
    
    // Draw buttons with classic Windows style
    for (const auto& buttonVisual : buttonVisuals)
    {
        bool isPressed = cachedState.buttons[buttonVisual.buttonIndex];
        drawClassicButton(g, buttonVisual.bounds, isPressed);
        
        g.setColour(juce::Colours::black);
        g.setFont(juce::Font("MS Sans Serif", 11.0f, juce::Font::plain));
        g.drawText(buttonVisual.name, 
                  buttonVisual.bounds, 
                  juce::Justification::centred, 
                  false);
    }
    
    // Draw touchpad
    g.setColour(cachedState.touchpad.pressed ? juce::Colours::orange.withAlpha(0.8f) : juce::Colours::darkgrey.brighter(0.15f));
    g.fillRoundedRectangle(touchpadBounds, 5.0f);
    g.setColour(juce::Colours::lightgrey);
    g.drawRoundedRectangle(touchpadBounds, 5.0f, 1.0f);
    
    // Draw touchpad label
    g.setColour(juce::Colours::white);
    g.setFont(14.0f);
    g.drawText("Touchpad", 
               touchpadBounds, 
               juce::Justification::centred, 
               false);
               
    // Show touchpad interaction if active
    if (cachedState.touchpad.touched)
    {
        // Calculate touch position within the touchpad bounds
        float touchX = touchpadBounds.getX() + (cachedState.touchpad.x * touchpadBounds.getWidth());
        float touchY = touchpadBounds.getY() + (cachedState.touchpad.y * touchpadBounds.getHeight());
        
        // Draw touch indicator
        float touchSize = 15.0f + (cachedState.touchpad.pressure * 10.0f); // Size varies with pressure
        g.setColour(juce::Colours::orange.withAlpha(0.7f));
        g.fillEllipse(touchX - touchSize/2, touchY - touchSize/2, touchSize, touchSize);
        
        // Display touch coordinates
        juce::String touchText = juce::String::formatted("X: %.2f Y: %.2f P: %.2f", 
                                                       cachedState.touchpad.x,
                                                       cachedState.touchpad.y,
                                                       cachedState.touchpad.pressure);
        g.setColour(juce::Colours::white);
        g.setFont(12.0f);
        g.drawText(touchText, 
                   touchpadBounds.getX(), 
                   touchpadBounds.getBottom() + 5.0f, 
                   touchpadBounds.getWidth(), 
                   20.0f, 
                   juce::Justification::centred, 
                   false);
    }

    // Draw gyroscope data if enabled
    if (cachedState.gyroscope.enabled)
    {
        // Create a box for gyroscope visualization below the touchpad
        auto gyroArea = juce::Rectangle<float>(
            touchpadBounds.getX(),
            touchpadBounds.getBottom() + 40.0f,
            touchpadBounds.getWidth(),
            60.0f
        );

        // Draw gyroscope background
        g.setColour(juce::Colours::darkgrey.brighter(0.1f));
        g.fillRoundedRectangle(gyroArea, 5.0f);
        g.setColour(juce::Colours::lightgrey);
        g.drawRoundedRectangle(gyroArea, 5.0f, 1.0f);

        // Draw gyroscope label
        g.setColour(juce::Colours::white);
        g.setFont(14.0f);
        g.drawText("Gyroscope", 
                   gyroArea.removeFromTop(20.0f), 
                   juce::Justification::centred, 
                   false);

        // Display gyroscope values
        juce::String gyroText = juce::String::formatted("X: %.2f Y: %.2f Z: %.2f rad/s",
                                                       cachedState.gyroscope.x,
                                                       cachedState.gyroscope.y,
                                                       cachedState.gyroscope.z);
        g.setColour(juce::Colours::white);
        g.setFont(12.0f);
        g.drawText(gyroText,
                   gyroArea,
                   juce::Justification::centred,
                   false);

        // Draw rotation indicators
        float indicatorWidth = gyroArea.getWidth() / 3.0f;
        float maxRotation = 10.0f; // Maximum rotation rate to visualize (rad/s)

        // X axis (roll)
        float xNormalized = juce::jlimit(-1.0f, 1.0f, cachedState.gyroscope.x / maxRotation);
        g.setColour(juce::Colours::red.withAlpha(0.8f));
        g.fillRect(gyroArea.getX(), gyroArea.getCentreY() - 2.0f,
                  indicatorWidth * (xNormalized + 1.0f) / 2.0f, 4.0f);

        // Y axis (pitch)
        float yNormalized = juce::jlimit(-1.0f, 1.0f, cachedState.gyroscope.y / maxRotation);
        g.setColour(juce::Colours::green.withAlpha(0.8f));
        g.fillRect(gyroArea.getX() + indicatorWidth, gyroArea.getCentreY() - 2.0f,
                  indicatorWidth * (yNormalized + 1.0f) / 2.0f, 4.0f);

        // Z axis (yaw)
        float zNormalized = juce::jlimit(-1.0f, 1.0f, cachedState.gyroscope.z / maxRotation);
        g.setColour(juce::Colours::blue.withAlpha(0.8f));
        g.fillRect(gyroArea.getX() + indicatorWidth * 2.0f, gyroArea.getCentreY() - 2.0f,
                  indicatorWidth * (zNormalized + 1.0f) / 2.0f, 4.0f);
    }
}

// Helper method to draw classic Windows style button
void GamepadComponent::drawClassicButton(juce::Graphics& g, const juce::Rectangle<float>& bounds, bool isPressed)
{
    g.setColour(juce::Colour(192, 192, 192));  // System gray
    g.fillRect(bounds);
    
    if (isPressed)
    {
        // Pressed state - inset effect
        g.setColour(juce::Colours::darkgrey);
        g.drawLine(bounds.getX(), bounds.getY(), bounds.getRight(), bounds.getY(), 1.0f);  // Top
        g.drawLine(bounds.getX(), bounds.getY(), bounds.getX(), bounds.getBottom(), 1.0f);  // Left
        
        g.setColour(juce::Colours::white);
        g.drawLine(bounds.getRight(), bounds.getY(), bounds.getRight(), bounds.getBottom(), 1.0f);  // Right
        g.drawLine(bounds.getX(), bounds.getBottom(), bounds.getRight(), bounds.getBottom(), 1.0f);  // Bottom
    }
    else
    {
        // Normal state - raised effect
        g.setColour(juce::Colours::white);
        g.drawLine(bounds.getX(), bounds.getY(), bounds.getRight(), bounds.getY(), 1.0f);  // Top
        g.drawLine(bounds.getX(), bounds.getY(), bounds.getX(), bounds.getBottom(), 1.0f);  // Left
        
        g.setColour(juce::Colours::darkgrey);
        g.drawLine(bounds.getRight(), bounds.getY(), bounds.getRight(), bounds.getBottom(), 1.0f);  // Right
        g.drawLine(bounds.getX(), bounds.getBottom(), bounds.getRight(), bounds.getBottom(), 1.0f);  // Bottom
    }
}

// Helper method to draw classic Windows style inset panel
void GamepadComponent::drawClassicInsetPanel(juce::Graphics& g, const juce::Rectangle<float>& bounds)
{
    g.setColour(juce::Colours::white);
    g.fillRect(bounds);
    
    g.setColour(juce::Colours::darkgrey);
    g.drawLine(bounds.getX(), bounds.getY(), bounds.getRight(), bounds.getY(), 1.0f);  // Top
    g.drawLine(bounds.getX(), bounds.getY(), bounds.getX(), bounds.getBottom(), 1.0f);  // Left
    
    g.setColour(juce::Colours::white);
    g.drawLine(bounds.getRight(), bounds.getY(), bounds.getRight(), bounds.getBottom(), 1.0f);  // Right
    g.drawLine(bounds.getX(), bounds.getBottom(), bounds.getRight(), bounds.getBottom(), 1.0f);  // Bottom
}

// Helper method to draw classic Windows style group box
void GamepadComponent::drawClassicGroupBox(juce::Graphics& g, const juce::Rectangle<float>& bounds, const juce::String& text)
{
    // Draw the group box border with classic Windows style
    g.setColour(juce::Colours::darkgrey);
    g.drawRect(bounds, 1.0f);
    
    // Draw the text
    g.setColour(juce::Colours::black);
    g.setFont(juce::Font("MS Sans Serif", 11.0f, juce::Font::plain));
    g.drawText(text, bounds.reduced(5), juce::Justification::centredLeft, false);
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