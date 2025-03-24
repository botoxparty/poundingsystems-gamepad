#include "ModernGamepadComponent.h"

ModernGamepadComponent::ModernGamepadComponent(const GamepadManager::GamepadState& state)
    : gamepadState(state)
    , cachedState(state)
    , leftStick("Left Stick", true)   // true indicates it's a stick (not a trigger)
    , rightStick("Right Stick", true)
{
    setupComponents();
    setupCallbacks();
    startTimer(33); // ~30fps refresh rate
}

ModernGamepadComponent::~ModernGamepadComponent()
{
    stopTimer();
}

void ModernGamepadComponent::setupComponents()
{
    // Add and make visible all child components
    addAndMakeVisible(shoulderSection);
    addAndMakeVisible(dPad);
    addAndMakeVisible(faceButtons);
    addAndMakeVisible(leftStick);
    addAndMakeVisible(rightStick);
    addAndMakeVisible(touchPad);
    addAndMakeVisible(gyroscope);
    
    // Setup Learn Mode button
    addAndMakeVisible(learnModeButton);
    learnModeButton.setButtonText("Learn Mode");
    learnModeButton.onClick = [this] { setMidiLearnMode(!midiLearnMode); };
    
    // Setup status label
    addAndMakeVisible(statusLabel);
    statusLabel.setJustificationType(juce::Justification::centredLeft);
}

void ModernGamepadComponent::setupLayout()
{
    auto bounds = getLocalBounds();
    
    // Setup status area at the top
    auto statusArea = bounds.removeFromTop(40);
    statusLabel.setBounds(statusArea.reduced(5).removeFromLeft(statusArea.getWidth() - 110));
    learnModeButton.setBounds(statusArea.reduced(5).removeFromRight(100));
    
    // Configure main layout
    mainLayout.flexDirection = juce::FlexBox::Direction::column;
    mainLayout.justifyContent = juce::FlexBox::JustifyContent::spaceBetween;
    mainLayout.alignItems = juce::FlexBox::AlignItems::stretch;
    
    // Configure row layouts
    topRow.flexDirection = juce::FlexBox::Direction::row;
    topRow.justifyContent = juce::FlexBox::JustifyContent::spaceBetween;
    topRow.alignItems = juce::FlexBox::AlignItems::center;
    
    middleRow.flexDirection = juce::FlexBox::Direction::row;
    middleRow.justifyContent = juce::FlexBox::JustifyContent::spaceBetween;
    middleRow.alignItems = juce::FlexBox::AlignItems::center;
    
    bottomRow.flexDirection = juce::FlexBox::Direction::row;
    bottomRow.justifyContent = juce::FlexBox::JustifyContent::spaceBetween;
    bottomRow.alignItems = juce::FlexBox::AlignItems::center;
    
    // Clear any existing items
    topRow.items.clear();
    middleRow.items.clear();
    bottomRow.items.clear();
    mainLayout.items.clear();
    
    // Add items to top row (shoulder buttons and triggers)
    topRow.items.add(juce::FlexItem(shoulderSection).withFlex(1.0f).withMargin(5.0f));
    
    // Add items to middle row (d-pad, touchpad, face buttons)
    middleRow.items.add(juce::FlexItem(dPad).withWidth(200).withMargin(5.0f));
    middleRow.items.add(juce::FlexItem(touchPad).withWidth(300).withMargin(5.0f));
    middleRow.items.add(juce::FlexItem(faceButtons).withWidth(200).withMargin(5.0f));
    
    // Add items to bottom row (left stick, gyroscope, right stick)
    bottomRow.items.add(juce::FlexItem(leftStick).withWidth(150).withMargin(5.0f));
    bottomRow.items.add(juce::FlexItem(gyroscope).withWidth(300).withMargin(5.0f));
    bottomRow.items.add(juce::FlexItem(rightStick).withWidth(150).withMargin(5.0f));
    
    // Add rows to main layout
    mainLayout.items.add(juce::FlexItem(topRow).withFlex(1.0f));
    mainLayout.items.add(juce::FlexItem(middleRow).withFlex(1.5f));
    mainLayout.items.add(juce::FlexItem(bottomRow).withFlex(1.0f));
    
    // Perform layout
    mainLayout.performLayout(bounds.toFloat());
}

void ModernGamepadComponent::setupCallbacks()
{
    // ShoulderSection callbacks
    shoulderSection.onButtonClick = [this](const juce::String& button) {
        if (button == "L1") sendMidiCC(29, 1.0f);  // CC 29 for L1
        else if (button == "R1") sendMidiCC(30, 1.0f);  // CC 30 for R1
        else if (button == "L2") sendMidiCC(5, 1.0f);   // CC 5 for L2
        else if (button == "R2") sendMidiCC(6, 1.0f);   // CC 6 for R2
    };
    
    // D-pad callbacks
    dPad.onButtonClick = [this](const juce::String& button) {
        if (button == "Up") sendMidiCC(31, 1.0f);      // CC 31 for Up
        else if (button == "Down") sendMidiCC(32, 1.0f);  // CC 32 for Down
        else if (button == "Left") sendMidiCC(33, 1.0f);  // CC 33 for Left
        else if (button == "Right") sendMidiCC(34, 1.0f); // CC 34 for Right
    };
    
    // Face buttons callbacks
    faceButtons.onButtonClick = [this](const juce::String& button) {
        if (button == "A") sendMidiCC(20, 1.0f);      // CC 20 for A
        else if (button == "B") sendMidiCC(21, 1.0f);  // CC 21 for B
        else if (button == "X") sendMidiCC(22, 1.0f);  // CC 22 for X
        else if (button == "Y") sendMidiCC(23, 1.0f);  // CC 23 for Y
    };
    
    // Analog sticks callbacks
    leftStick.onAxisChange = [this](const juce::String& axis, float value) {
        if (axis == "X") sendMidiCC(1, (value + 1.0f) * 0.5f);  // CC 1 for Left X
        else if (axis == "Y") sendMidiCC(2, (value + 1.0f) * 0.5f);  // CC 2 for Left Y
    };
    
    rightStick.onAxisChange = [this](const juce::String& axis, float value) {
        if (axis == "X") sendMidiCC(3, (value + 1.0f) * 0.5f);  // CC 3 for Right X
        else if (axis == "Y") sendMidiCC(4, (value + 1.0f) * 0.5f);  // CC 4 for Right Y
    };
    
    // Touchpad callbacks
    touchPad.onValueChange = [this](float x, float y, float pressure) {
        sendMidiCC(35, x);          // CC 35 for Touch X
        sendMidiCC(36, y);          // CC 36 for Touch Y
        sendMidiCC(37, pressure);    // CC 37 for Touch Pressure
    };
    
    // Gyroscope callbacks
    gyroscope.onValueChange = [this](float x, float y, float z) {
        sendMidiCC(38, (x + 1.0f) * 0.5f);  // CC 38 for Gyro X
        sendMidiCC(39, (y + 1.0f) * 0.5f);  // CC 39 for Gyro Y
        sendMidiCC(40, (z + 1.0f) * 0.5f);  // CC 40 for Gyro Z
    };
}

void ModernGamepadComponent::paint(juce::Graphics& g)
{
    // Fill background
    g.fillAll(juce::Colours::white);
    
    // Draw border
    g.setColour(juce::Colours::darkgrey);
    g.drawRect(getLocalBounds(), 1);
}

void ModernGamepadComponent::resized()
{
    setupLayout();
}

void ModernGamepadComponent::updateState(const GamepadManager::GamepadState& newState) {
    if (!midiLearnMode) {
        // Update shoulder section
        shoulderSection.setState({
            newState.buttons[9],  // L1
            newState.buttons[10], // R1
            juce::jlimit(0.0f, 1.0f, (newState.axes[4] + 1.0f) * 0.5f),  // L2
            juce::jlimit(0.0f, 1.0f, (newState.axes[5] + 1.0f) * 0.5f)   // R2
        });
        
        // Update D-pad
        dPad.setState({
            newState.buttons[11], // Up
            newState.buttons[12], // Down
            newState.buttons[13], // Left
            newState.buttons[14]  // Right
        });
        
        // Update face buttons
        faceButtons.setState({
            newState.buttons[0],  // A
            newState.buttons[1],  // B
            newState.buttons[2],  // X
            newState.buttons[3]   // Y
        });

        // Update analog sticks
        {
            AnalogStick::State stickState;
            stickState.isEnabled = true;
            stickState.xValue = juce::jlimit(-1.0f, 1.0f, newState.axes[0]);
            stickState.yValue = juce::jlimit(-1.0f, 1.0f, newState.axes[1]);
            stickState.isPressed = newState.buttons[7];
            stickState.xCC = 1;
            stickState.yCC = 2;
            stickState.pressCC = 29;
            stickState.isLearnMode = false;
            stickState.name = "Left Stick";
            stickState.isStick = true;
            leftStick.setState(stickState);
        }

        {
            AnalogStick::State stickState;
            stickState.isEnabled = true;
            stickState.xValue = juce::jlimit(-1.0f, 1.0f, newState.axes[2]);
            stickState.yValue = juce::jlimit(-1.0f, 1.0f, newState.axes[3]);
            stickState.isPressed = newState.buttons[8];
            stickState.xCC = 3;
            stickState.yCC = 4;
            stickState.pressCC = 30;
            stickState.isLearnMode = false;
            stickState.name = "Right Stick";
            stickState.isStick = true;
            rightStick.setState(stickState);
        }

        // Update touchpad
        {
            TouchPad::State padState;
            padState.isEnabled = true;
            padState.xValue = juce::jlimit(0.0f, 1.0f, newState.touchpad.x);
            padState.yValue = juce::jlimit(0.0f, 1.0f, newState.touchpad.y);
            padState.pressure = juce::jlimit(0.0f, 1.0f, newState.touchpad.pressure);
            padState.isPressed = newState.touchpad.pressed;
            padState.xCC = 35;
            padState.yCC = 36;
            padState.pressureCC = 37;
            padState.isLearnMode = false;
            touchPad.setState(padState);
        }

        // Update gyroscope
        {
            Gyroscope::State gyroState;
            gyroState.enabled = newState.gyroscope.enabled;
            gyroState.x = juce::jlimit(-1.0f, 1.0f, newState.gyroscope.x);
            gyroState.y = juce::jlimit(-1.0f, 1.0f, newState.gyroscope.y);
            gyroState.z = juce::jlimit(-1.0f, 1.0f, newState.gyroscope.z);
            gyroState.xCC = 40;
            gyroState.yCC = 41;
            gyroState.zCC = 42;
            gyroState.isLearnMode = false;
            gyroscope.setState(gyroState);
        }
    }

    // Update status label
    statusLabel.setText(newState.connected
        ? "Connected: " + newState.name + (midiLearnMode ? " (Learn Mode)" : "")
        : "Disconnected",
        juce::dontSendNotification);

    cachedState = newState;
    repaint();
}

void ModernGamepadComponent::setMidiLearnMode(bool enabled)
{
    if (midiLearnMode != enabled)
    {
        midiLearnMode = enabled;
        learnModeButton.setToggleState(enabled, juce::dontSendNotification);
        
        // Update learn mode state for all components
        shoulderSection.setLearnMode(enabled);
        dPad.setLearnMode(enabled);
        faceButtons.setLearnMode(enabled);
        leftStick.setLearnMode(enabled);
        rightStick.setLearnMode(enabled);
        touchPad.setLearnMode(enabled);
        gyroscope.setLearnMode(enabled);
        
        repaint();
    }
}

void ModernGamepadComponent::sendMidiCC(int ccNumber, float value)
{
    // Map value from 0.0-1.0 to 0-127
    int midiValue = static_cast<int>(value * 127.0f);
    MidiOutputManager::getInstance().sendControlChange(1, ccNumber, midiValue);
}

void ModernGamepadComponent::timerCallback()
{
    updateState(gamepadState);
} 