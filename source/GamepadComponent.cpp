#include "GamepadComponent.h"

GamepadComponent::GamepadComponent(const GamepadManager::GamepadState& state)
    : gamepadState(state), cachedState(state)
{
    setupVisuals();
    setupMidiLearnControls();
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
    
    // Touchpad (as a button)
    buttonVisuals.push_back({"Touch", {}, 15});
    
    // Axes
    axisVisuals.push_back({"Left Stick", {}, 0, true});  // Represents both X and Y axes
    axisVisuals.push_back({"Right Stick", {}, 2, true}); // Represents both X and Y axes
    axisVisuals.push_back({"L2", {}, 4, false});
    axisVisuals.push_back({"R2", {}, 5, false});
}

void GamepadComponent::setupMidiLearnControls()
{
    // Starting CC number for controls
    int nextCC = 35; // Start after the button CCs (20-34)
    
    // Add gyroscope controls
    midiLearnControls.push_back({"Gyro X", {}, nextCC++, true, -1, 0});
    midiLearnControls.push_back({"Gyro Y", {}, nextCC++, true, -1, 1});
    midiLearnControls.push_back({"Gyro Z", {}, nextCC++, true, -1, 2});
    
    // Add touchpad controls
    midiLearnControls.push_back({"Touch X", {}, nextCC++, true, -1, 3});
    midiLearnControls.push_back({"Touch Y", {}, nextCC++, true, -1, 4});
    midiLearnControls.push_back({"Touch Pressure", {}, nextCC++, true, -1, 5});
    
    // Add stick controls - using CC 1-4 for sticks
    midiLearnControls.push_back({"Left Stick X", {}, 1, true, 0, 0});
    midiLearnControls.push_back({"Left Stick Y", {}, 2, true, 0, 1});
    midiLearnControls.push_back({"Right Stick X", {}, 3, true, 2, 0});
    midiLearnControls.push_back({"Right Stick Y", {}, 4, true, 2, 1});
    
    // Add trigger controls - using CC 5-6 for triggers
    midiLearnControls.push_back({"L2", {}, 5, true, 4, 0});
    midiLearnControls.push_back({"R2", {}, 6, true, 5, 0});
    
    // Add button controls - using CC 20-34 for buttons
    for (const auto& buttonVisual : buttonVisuals)
    {
        midiLearnControls.push_back({
            buttonVisual.name,
            buttonVisual.bounds,
            buttonVisual.buttonIndex + 20, // CC numbers start at 20
            false,
            buttonVisual.buttonIndex,
            0
        });
    }
}

void GamepadComponent::setMidiLearnMode(bool enabled)
{
    if (midiLearnMode != enabled)
    {
        midiLearnMode = enabled;
        repaint();
    }
}

juce::Rectangle<float> GamepadComponent::calculateTouchpadBounds()
{
    auto bounds = getLocalBounds().toFloat();
    auto middleArea = bounds;
    auto centerArea = middleArea.removeFromTop(middleArea.getHeight() * 0.5f);
    auto touchpadArea = centerArea.removeFromTop(centerArea.getHeight() * 0.5f);
    return juce::Rectangle<float>(
        touchpadArea.getCentreX() - 100.0f,
        touchpadArea.getCentreY() - 15.0f,
        200.0f,    // width
        60.0f      // height
    );
}

void GamepadComponent::mouseDown(const juce::MouseEvent& event)
{
    auto point = event.position.toFloat();
    
    // Check if Learn Mode button was clicked
    if (midiLearnButtonBounds.contains(point))
    {
        setMidiLearnMode(!midiLearnMode);
        return;
    }
    
    // Only handle MIDI learn clicks when in learn mode
    if (midiLearnMode)
    {
        // Set touchpad button bounds
        buttonVisuals[15].bounds = calculateTouchpadBounds();
        
        // First check buttons (including touchpad)
        for (const auto& buttonVisual : buttonVisuals)
        {
            if (buttonVisual.bounds.contains(point))
            {
                // Find corresponding MIDI learn control
                for (auto& control : midiLearnControls)
                {
                    if (!control.isAxis && control.index == buttonVisual.buttonIndex)
                    {
                        sendMidiLearnCC(control.ccNumber, 1.0f);
                        return;
                    }
                }
            }
        }
        
        // Check touchpad X/Y/Pressure buttons
        float labelButtonWidth = buttonVisuals[15].bounds.getWidth() / 3.0f;
        float labelButtonHeight = 20.0f;
        float labelY = buttonVisuals[15].bounds.getY() - labelButtonHeight - 5.0f;
        
        // X button (left)
        auto xButtonBounds = juce::Rectangle<float>(buttonVisuals[15].bounds.getX(), labelY, labelButtonWidth, labelButtonHeight);
        // Y button (middle)
        auto yButtonBounds = juce::Rectangle<float>(buttonVisuals[15].bounds.getX() + labelButtonWidth, labelY, labelButtonWidth, labelButtonHeight);
        // Pressure button (right)
        auto pressureButtonBounds = juce::Rectangle<float>(buttonVisuals[15].bounds.getX() + labelButtonWidth * 2, labelY, labelButtonWidth, labelButtonHeight);
        
        // Check if touchpad buttons were clicked
        for (auto& control : midiLearnControls)
        {
            if (control.isAxis && control.index == -1)
            {
                if (control.subIndex == 3 && xButtonBounds.contains(point))
                {
                    sendMidiLearnCC(control.ccNumber, cachedState.touchpad.x);
                    return;
                }
                else if (control.subIndex == 4 && yButtonBounds.contains(point))
                {
                    sendMidiLearnCC(control.ccNumber, cachedState.touchpad.y);
                    return;
                }
                else if (control.subIndex == 5 && pressureButtonBounds.contains(point))
                {
                    sendMidiLearnCC(control.ccNumber, cachedState.touchpad.pressure);
                    return;
                }
            }
        }
        
        // Check if touchpad itself was clicked for pressed state
        if (buttonVisuals[15].bounds.contains(point))
        {
            for (auto& control : midiLearnControls)
            {
                if (!control.isAxis && control.index == -1 && control.subIndex == 6)
                {
                    sendMidiLearnCC(control.ccNumber, cachedState.touchpad.pressed ? 1.0f : 0.0f);
                    return;
                }
            }
        }
        
        // Then check axes and their X/Y buttons
        for (const auto& axisVisual : axisVisuals)
        {
            if (axisVisual.isStick)
            {
                // Calculate X/Y button bounds
                float labelButtonWidth = axisVisual.bounds.getWidth() * 0.4f;
                float labelButtonHeight = 20.0f;
                float labelY = axisVisual.bounds.getY() - labelButtonHeight - 5.0f;
                
                // X button bounds (left)
                auto xButtonBounds = juce::Rectangle<float>(axisVisual.bounds.getX(), labelY, labelButtonWidth, labelButtonHeight);
                // Y button bounds (right)
                auto yButtonBounds = juce::Rectangle<float>(axisVisual.bounds.getRight() - labelButtonWidth, labelY, labelButtonWidth, labelButtonHeight);

                // Check if X or Y button was clicked
                for (auto& control : midiLearnControls)
                {
                    if (control.isAxis && control.index == axisVisual.axisIndex)
                    {
                        if (control.subIndex == 0 && xButtonBounds.contains(point))
                        {
                            sendMidiLearnCC(control.ccNumber, 1.0f);
                            return;
                        }
                        else if (control.subIndex == 1 && yButtonBounds.contains(point))
                        {
                            sendMidiLearnCC(control.ccNumber, 1.0f);
                            return;
                        }
                    }
                }
            }
            else if (axisVisual.bounds.contains(point)) // Triggers
            {
                for (auto& control : midiLearnControls)
                {
                    if (control.isAxis && control.index == axisVisual.axisIndex)
                    {
                        sendMidiLearnCC(control.ccNumber, 1.0f);
                        return;
                    }
                }
            }
        }
    }
}

GamepadComponent::MidiLearnControl* GamepadComponent::findMidiLearnControlAt(juce::Point<float> point)
{
    for (auto& control : midiLearnControls)
    {
        if (control.bounds.contains(point))
            return &control;
    }
    return nullptr;
}

void GamepadComponent::sendMidiLearnCC(int ccNumber, float value)
{
    // Map value from 0.0-1.0 to 0-127
    int midiValue = static_cast<int>(value * 127.0f);
    MidiOutputManager::getInstance().sendControlChange(1, ccNumber, midiValue);
}

void GamepadComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    
    // White background to contrast with parent's light gray
    g.setColour(juce::Colours::white);
    g.fillAll();
    
    // Simple border
    g.setColour(juce::Colours::darkgrey);
    g.drawRect(getLocalBounds().toFloat(), 1.0f);
    
    // Draw connection status and Learn Mode button
    g.setColour(juce::Colours::black);
    g.setFont(juce::Font("MS Sans Serif", 12.0f, juce::Font::plain));
    
    auto statusArea = bounds.removeFromTop(30.0f).reduced(5);
    auto midiLearnButtonArea = statusArea.removeFromRight(100.0f);
    midiLearnButtonArea.removeFromLeft(5.0f); // Add 5px space between button and status
    
    // Draw Learn Mode button
    drawClassicButton(g, midiLearnButtonArea, midiLearnMode);
    g.setColour(juce::Colours::black);
    g.drawText("Learn Mode", midiLearnButtonArea, juce::Justification::centred, false);
    
    // Store button bounds for click handling
    midiLearnButtonBounds = midiLearnButtonArea;
    
    juce::String statusText = cachedState.connected
        ? "Connected: " + cachedState.name + (midiLearnMode ? " (Learn Mode)" : "")
        : "Disconnected";
    
    // Draw status text in a classic Windows style group box
    drawClassicGroupBox(g, statusArea, statusText);
    
    if (!cachedState.connected)
    {
        // Draw connection message in the center
        g.setColour(juce::Colours::black);
        g.setFont(juce::Font("MS Sans Serif", 14.0f, juce::Font::plain));
        g.drawText("Connect your game controller via USB or Bluetooth",
                  bounds,
                  juce::Justification::centred,
                  true);
        return;
    }
    
    // Set touchpad button bounds
    buttonVisuals[15].bounds = calculateTouchpadBounds();
    
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
    buttonVisuals[15].bounds = juce::Rectangle<float>(
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
    
    // Create gyroscope area first so we can use it for menu positioning
    auto gyroArea = juce::Rectangle<float>(
        buttonVisuals[15].bounds.getX(),
        buttonVisuals[15].bounds.getBottom() + 60.0f,
        buttonVisuals[15].bounds.getWidth(),
        60.0f
    );
    
    // Position menu buttons centered between touchpad and gyroscope
    float menuY = buttonVisuals[15].bounds.getBottom() + ((gyroArea.getY() - buttonVisuals[15].bounds.getBottom()) / 2) - (menuButtonHeight / 2);
    
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
            
            // Find corresponding MIDI learn controls for X and Y
            int ccNumberX = -1;
            int ccNumberY = -1;
            for (const auto& control : midiLearnControls)
            {
                if (control.isAxis && control.index == axisVisual.axisIndex)
                {
                    if (control.subIndex == 0)
                        ccNumberX = control.ccNumber;
                    else if (control.subIndex == 1)
                        ccNumberY = control.ccNumber;
                }
            }
            
            // Draw stick with classic button style and MIDI info
            auto stickBounds = juce::Rectangle<float>(stickX - buttonSize/2, stickY - buttonSize/2, buttonSize, buttonSize);
            drawClassicButton(g, stickBounds, true);
            
            // Create X/Y label buttons above the stick
            float labelButtonWidth = axisVisual.bounds.getWidth() * 0.4f;
            float labelButtonHeight = 20.0f;
            float labelY = axisVisual.bounds.getY() - labelButtonHeight - 5.0f;
            
            // X button (left)
            auto xButtonBounds = juce::Rectangle<float>(axisVisual.bounds.getX(), labelY, labelButtonWidth, labelButtonHeight);
            juce::String xText = "X: " + juce::String(x, 2);
            drawClassicButton(g, xButtonBounds, false, xText, ccNumberX);
            
            // Y button (right)
            auto yButtonBounds = juce::Rectangle<float>(axisVisual.bounds.getRight() - labelButtonWidth, labelY, labelButtonWidth, labelButtonHeight);
            juce::String yText = "Y: " + juce::String(y, 2);
            drawClassicButton(g, yButtonBounds, false, yText, ccNumberY);
            
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
            
            // Find corresponding MIDI learn control
            int ccNumber = -1;
            for (const auto& control : midiLearnControls)
            {
                if (control.isAxis && control.index == axisVisual.axisIndex)
                {
                    ccNumber = control.ccNumber;
                    break;
                }
            }
            
            // Draw trigger name and CC number if in learn mode
            g.setColour(juce::Colours::black);
            g.setFont(juce::Font("MS Sans Serif", 11.0f, juce::Font::plain));
            juce::String labelText = axisVisual.name;
            if (midiLearnMode && ccNumber >= 0)
            {
                labelText += " (CC" + juce::String(ccNumber) + ")";
            }
            g.drawText(labelText, 
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
        
        // Find corresponding MIDI learn control
        int ccNumber = -1;
        for (const auto& control : midiLearnControls)
        {
            if (!control.isAxis && control.index == buttonVisual.buttonIndex)
            {
                ccNumber = control.ccNumber;
                break;
            }
        }
        
        drawClassicButton(g, buttonVisual.bounds, isPressed, buttonVisual.name, ccNumber);
    }
    
    // Draw touchpad
    const auto& touchArea = buttonVisuals[15].bounds;
    
    // Find touchpad button's CC number
    int ccNumber = -1;
    for (const auto& control : midiLearnControls)
    {
        if (!control.isAxis && control.index == buttonVisuals[15].buttonIndex)
        {
            ccNumber = control.ccNumber;
            break;
        }
    }
    
    // In learn mode with a CC number, use blue background
    if (midiLearnMode && ccNumber >= 0)
    {
        g.setColour(juce::Colours::blue.withAlpha(0.7f));
        g.fillRoundedRectangle(touchArea, 5.0f);
        g.setColour(juce::Colours::white);
        g.drawRoundedRectangle(touchArea, 5.0f, 1.0f);
        
        // Draw CC number
        g.setFont(juce::Font("MS Sans Serif", 11.0f, juce::Font::plain));
        g.drawText("CC" + juce::String(ccNumber), touchArea, juce::Justification::centred, false);
    }
    else
    {
        g.setColour(cachedState.touchpad.pressed ? juce::Colours::orange.withAlpha(0.8f) : juce::Colours::darkgrey.brighter(0.15f));
        g.fillRoundedRectangle(touchArea, 5.0f);
        g.setColour(juce::Colours::lightgrey);
        g.drawRoundedRectangle(touchArea, 5.0f, 1.0f);
    }
    
    // Create X/Y/Pressure label buttons above the touchpad
    float labelButtonWidth = touchArea.getWidth() / 3.0f;
    float labelButtonHeight = 20.0f;
    float labelY = touchArea.getY() - labelButtonHeight - 5.0f;
    
    // X button (left)
    auto xButtonBounds = juce::Rectangle<float>(touchArea.getX(), labelY, labelButtonWidth, labelButtonHeight);
    juce::String xText = "X: " + juce::String(cachedState.touchpad.x, 2);
    int ccNumberX = -1;
    for (const auto& control : midiLearnControls)
    {
        if (control.isAxis && control.index == -1 && control.subIndex == 3)
        {
            ccNumberX = control.ccNumber;
            break;
        }
    }
    drawClassicButton(g, xButtonBounds, false, xText, ccNumberX);
    
    // Y button (middle)
    auto yButtonBounds = juce::Rectangle<float>(touchArea.getX() + labelButtonWidth, labelY, labelButtonWidth, labelButtonHeight);
    juce::String yText = "Y: " + juce::String(cachedState.touchpad.y, 2);
    int ccNumberY = -1;
    for (const auto& control : midiLearnControls)
    {
        if (control.isAxis && control.index == -1 && control.subIndex == 4)
        {
            ccNumberY = control.ccNumber;
            break;
        }
    }
    drawClassicButton(g, yButtonBounds, false, yText, ccNumberY);
    
    // Pressure button (right)
    auto pressureButtonBounds = juce::Rectangle<float>(touchArea.getX() + labelButtonWidth * 2, labelY, labelButtonWidth, labelButtonHeight);
    juce::String pressureText = "P: " + juce::String(cachedState.touchpad.pressure, 2);
    int ccNumberPressure = -1;
    for (const auto& control : midiLearnControls)
    {
        if (control.isAxis && control.index == -1 && control.subIndex == 5)
        {
            ccNumberPressure = control.ccNumber;
            break;
        }
    }
    drawClassicButton(g, pressureButtonBounds, false, pressureText, ccNumberPressure);
    
    // Draw touchpad label
    g.setColour(juce::Colours::white);
    g.setFont(14.0f);
    g.drawText("Touchpad", 
               touchArea, 
               juce::Justification::centred, 
               false);
               
    // Show touchpad interaction if active
    if (cachedState.touchpad.touched)
    {
        // Calculate touch position within the touchpad bounds
        float touchX = touchArea.getX() + (cachedState.touchpad.x * touchArea.getWidth());
        float touchY = touchArea.getY() + (cachedState.touchpad.y * touchArea.getHeight());
        
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
                   touchArea.getX(), 
                   touchArea.getBottom() + 5.0f, 
                   touchArea.getWidth(), 
                   20.0f, 
                   juce::Justification::centred, 
                   false);
    }

    // Draw gyroscope data if enabled
    if (cachedState.gyroscope.enabled)
    {
        // Draw gyroscope background
        g.setColour(juce::Colours::darkgrey.brighter(0.1f));
        g.fillRoundedRectangle(gyroArea, 5.0f);
        g.setColour(juce::Colours::lightgrey);
        g.drawRoundedRectangle(gyroArea, 5.0f, 1.0f);

        // Draw gyroscope label
        g.setColour(juce::Colours::white);
        g.setFont(14.0f);
        auto headerArea = gyroArea.removeFromTop(20.0f);
        g.drawText("Gyroscope", 
                   headerArea, 
                   juce::Justification::centred, 
                   false);

        // Split the remaining area into three rows for each axis
        float rowHeight = gyroArea.getHeight() / 3.0f;
        float valueWidth = 80.0f; // Width for the value text
        float barHeight = 6.0f;   // Height of the indicator bar
        float maxRotation = 10.0f; // Maximum rotation rate to visualize (rad/s)
        
        // X axis (roll) - Red
        auto xRow = gyroArea.removeFromTop(rowHeight);
        auto xValueArea = xRow.removeFromLeft(valueWidth);
        auto xBarArea = xRow.withHeight(barHeight).withY(xRow.getCentreY() - barHeight/2);
        
        float xNormalized = juce::jlimit(-1.0f, 1.0f, cachedState.gyroscope.x / maxRotation);
        
        // Draw X label and value
        g.setColour(juce::Colours::red);
        g.setFont(12.0f);
        g.drawText("X: " + juce::String(cachedState.gyroscope.x, 2) + " rad/s",
                  xValueArea,
                  juce::Justification::centredLeft,
                  false);
        
        // Draw X bar
        g.fillRect(xBarArea.withWidth(xBarArea.getWidth() * (xNormalized + 1.0f) / 2.0f));
        
        // Y axis (pitch) - Green
        auto yRow = gyroArea.removeFromTop(rowHeight);
        auto yValueArea = yRow.removeFromLeft(valueWidth);
        auto yBarArea = yRow.withHeight(barHeight).withY(yRow.getCentreY() - barHeight/2);
        
        float yNormalized = juce::jlimit(-1.0f, 1.0f, cachedState.gyroscope.y / maxRotation);
        
        // Draw Y label and value
        g.setColour(juce::Colours::green);
        g.drawText("Y: " + juce::String(cachedState.gyroscope.y, 2) + " rad/s",
                  yValueArea,
                  juce::Justification::centredLeft,
                  false);
        
        // Draw Y bar
        g.fillRect(yBarArea.withWidth(yBarArea.getWidth() * (yNormalized + 1.0f) / 2.0f));
        
        // Z axis (yaw) - Blue
        auto zRow = gyroArea;
        auto zValueArea = zRow.removeFromLeft(valueWidth);
        auto zBarArea = zRow.withHeight(barHeight).withY(zRow.getCentreY() - barHeight/2);
        
        float zNormalized = juce::jlimit(-1.0f, 1.0f, cachedState.gyroscope.z / maxRotation);
        
        // Draw Z label and value
        g.setColour(juce::Colours::blue);
        g.drawText("Z: " + juce::String(cachedState.gyroscope.z, 2) + " rad/s",
                  zValueArea,
                  juce::Justification::centredLeft,
                  false);
        
        // Draw Z bar
        g.fillRect(zBarArea.withWidth(zBarArea.getWidth() * (zNormalized + 1.0f) / 2.0f));
    }
}

// Helper method to draw classic Windows style button
void GamepadComponent::drawClassicButton(juce::Graphics& g, const juce::Rectangle<float>& bounds, bool isPressed, const juce::String& text, int ccNumber)
{
    // In learn mode with a CC number, use blue background
    if (midiLearnMode && ccNumber >= 0)
    {
        g.setColour(juce::Colours::blue.withAlpha(0.7f));
        g.fillRect(bounds);
        g.setColour(juce::Colours::white);
        g.drawRect(bounds, 1.0f);
    }
    else
    {
        // Normal button style
        g.setColour(isPressed ? juce::Colours::grey : juce::Colour(220, 220, 220));
        g.fillRect(bounds);
        g.setColour(juce::Colours::darkgrey);
        g.drawRect(bounds, 1.0f);
    }

    // Draw text if provided
    if (text.isNotEmpty())
    {
        g.setColour(midiLearnMode && ccNumber >= 0 ? juce::Colours::white : juce::Colours::black);
        g.setFont(juce::Font("MS Sans Serif", 11.0f, juce::Font::plain));
        
        if (ccNumber >= 0 && midiLearnMode)
        {
            // In learn mode, show CC number prominently
            auto textBounds = bounds.reduced(2);
            g.drawText(text, textBounds.removeFromTop(textBounds.getHeight() * 0.6f), 
                      juce::Justification::centred, false);
            g.setFont(juce::Font("MS Sans Serif", 9.0f, juce::Font::plain));
            g.drawText("CC" + juce::String(ccNumber), textBounds, 
                      juce::Justification::centred, false);
        }
        else
        {
            // Normal mode, just show the button name
            g.drawText(text, bounds, juce::Justification::centred, false);
        }
    }
}

// Helper method to draw classic Windows style inset panel
void GamepadComponent::drawClassicInsetPanel(juce::Graphics& g, const juce::Rectangle<float>& bounds)
{
    // White background
    g.setColour(juce::Colours::white);
    g.fillRect(bounds);
    
    // Simple border
    g.setColour(juce::Colours::darkgrey);
    g.drawRect(bounds, 1.0f);
}

// Helper method to draw classic Windows style group box
void GamepadComponent::drawClassicGroupBox(juce::Graphics& g, const juce::Rectangle<float>& bounds, const juce::String& text)
{
    // Simple border
    g.setColour(juce::Colours::darkgrey);
    g.drawRect(bounds, 1.0f);
    
    // Draw the text
    g.setColour(juce::Colours::black);
    g.setFont(juce::Font(11.0f));
    g.drawText(text, bounds.reduced(5), juce::Justification::centredLeft, false);
}

void GamepadComponent::resized()
{
    
}

void GamepadComponent::updateState(const GamepadManager::GamepadState& newState)
{
    // Only send MIDI messages if not in learn mode
    if (!midiLearnMode)
    {
        if (newState.gyroscope.enabled && 
            (newState.gyroscope.x != cachedState.gyroscope.x ||
             newState.gyroscope.y != cachedState.gyroscope.y ||
             newState.gyroscope.z != cachedState.gyroscope.z))
        {
            sendGyroscopeMidiCC(newState.gyroscope);
        }
        
        if (newState.touchpad.touched &&
            (newState.touchpad.x != cachedState.touchpad.x ||
             newState.touchpad.y != cachedState.touchpad.y ||
             newState.touchpad.pressure != cachedState.touchpad.pressure))
        {
            sendTouchpadMidiCC(newState.touchpad);
        }
    }
    
    cachedState = newState;
    repaint();
}

void GamepadComponent::sendGyroscopeMidiCC(const GamepadManager::GamepadState::GyroscopeState& gyro)
{
    // Use MIDI channel 1
    const int channel = 1;
    
    // Map gyroscope values (-10 to 10 rad/s) to MIDI CC values (0-127)
    // Using CC numbers:
    // 20: X-axis
    // 21: Y-axis
    // 22: Z-axis
    
    auto mapGyroToCC = [](float value) {
        // Clamp value between -10 and 10 rad/s
        value = std::clamp(value, -10.0f, 10.0f);
        // Map to 0-127 range
        return static_cast<int>((value + 10.0f) * 127.0f / 20.0f);
    };
    
    auto& midiOutput = MidiOutputManager::getInstance();
    midiOutput.sendControlChange(channel, 20, mapGyroToCC(gyro.x));
    midiOutput.sendControlChange(channel, 21, mapGyroToCC(gyro.y));
    midiOutput.sendControlChange(channel, 22, mapGyroToCC(gyro.z));
}

void GamepadComponent::sendTouchpadMidiCC(const GamepadManager::GamepadState::TouchpadState& touchpad)
{
    // Use MIDI channel 1 (same as gyroscope)
    const int channel = 1;
    
    // Map touchpad values (0-1) to MIDI CC values (0-127)
    // Using CC numbers:
    // 23: X position
    // 24: Y position
    // 25: Pressure
    
    auto mapTouchpadToCC = [](float value) {
        return static_cast<int>(value * 127.0f);
    };
    
    auto& midiOutput = MidiOutputManager::getInstance();
    midiOutput.sendControlChange(channel, 23, mapTouchpadToCC(touchpad.x));
    midiOutput.sendControlChange(channel, 24, mapTouchpadToCC(touchpad.y));
    midiOutput.sendControlChange(channel, 25, mapTouchpadToCC(touchpad.pressure));
}

void GamepadComponent::timerCallback()
{
    // Update the visualization with the current state
    updateState(gamepadState);
} 