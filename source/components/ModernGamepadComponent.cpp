#include "ModernGamepadComponent.h"
#include "MidiCCMapping.h"

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
    addAndMakeVisible(gyroscopeDisplay);
    addAndMakeVisible(accelerometerDisplay);
    
    // Setup Learn Mode button
    addAndMakeVisible(learnModeButton);
    learnModeButton.setButtonText("Learn Mode");
    learnModeButton.onClick = [this] { setMidiLearnMode(!midiLearnMode); };
    learnModeButton.setColour(juce::TextButton::buttonColourId, juce::Colours::darkgrey);
    learnModeButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    learnModeButton.setColour(juce::TextButton::buttonOnColourId, juce::Colours::blue);
    
    // Setup status label
    addAndMakeVisible(statusLabel);
    statusLabel.setJustificationType(juce::Justification::centredLeft);
    statusLabel.setColour(juce::Label::textColourId, juce::Colours::black);
    statusLabel.setColour(juce::Label::backgroundColourId, juce::Colours::white);
    statusLabel.setColour(juce::Label::outlineColourId, juce::Colours::darkgrey);
    statusLabel.setFont(juce::Font(14.0f));
    statusLabel.setBorderSize(juce::BorderSize<int>(1, 10, 1, 1)); // Left: 10px, Others: 1px
}

void ModernGamepadComponent::setupLayout()
{
    auto bounds = getLocalBounds();
    
    // Setup status area at the top
    auto statusArea = bounds.removeFromTop(40);
    auto buttonArea = statusArea.reduced(5).removeFromRight(100);
    statusLabel.setBounds(statusArea.reduced(5).removeFromLeft(statusArea.getWidth() - 115)); // Extra 5px for gap
    learnModeButton.setBounds(buttonArea);
    
    // Configure main layout
    mainLayout.flexDirection = juce::FlexBox::Direction::column;
    mainLayout.justifyContent = juce::FlexBox::JustifyContent::flexStart;
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
    
    // Calculate component sizes
    float remainingHeight = bounds.getHeight() - 20; // -20 for padding
    float shoulderSectionHeight = 40; // Fixed height instead of relative
    float mainRowHeight = (remainingHeight - shoulderSectionHeight) / 2.0f; // Split remaining space between middle and bottom
    float shoulderSectionWidth = bounds.getWidth() - 20; // -20 for padding
    float dpadWidth = 200;
    float touchpadWidth = 300;
    float faceButtonsWidth = 200;
    float stickWidth = 150;
    float gyroscopeWidth = 150;
    
    // Add items to top row (shoulder buttons and triggers)
    topRow.items.add(juce::FlexItem(shoulderSection).withWidth(shoulderSectionWidth).withHeight(shoulderSectionHeight).withMargin(2.0f));
    
    // Add items to middle row (d-pad, touchpad, face buttons)
    middleRow.items.add(juce::FlexItem(dPad).withWidth(dpadWidth).withHeight(mainRowHeight).withMargin(5.0f));
    middleRow.items.add(juce::FlexItem(touchPad).withWidth(touchpadWidth).withHeight(mainRowHeight).withMargin(5.0f));
    middleRow.items.add(juce::FlexItem(faceButtons).withWidth(faceButtonsWidth).withHeight(mainRowHeight).withMargin(5.0f));
    
    // Add items to bottom row (left stick, sensors, right stick)
    bottomRow.items.add(juce::FlexItem(leftStick).withWidth(stickWidth).withHeight(mainRowHeight).withMargin(5.0f));
    
    // Create a container for the sensors
    juce::FlexBox sensorContainer;
    sensorContainer.flexDirection = juce::FlexBox::Direction::row;
    sensorContainer.justifyContent = juce::FlexBox::JustifyContent::spaceBetween;
    sensorContainer.alignItems = juce::FlexBox::AlignItems::stretch;
    
    // Add both sensors to the container with equal width
    sensorContainer.items.add(juce::FlexItem(gyroscopeDisplay).withFlex(1.0f).withMargin(2.0f));
    sensorContainer.items.add(juce::FlexItem(accelerometerDisplay).withFlex(1.0f).withMargin(2.0f));
    
    // Add the sensor container to the bottom row
    bottomRow.items.add(juce::FlexItem(sensorContainer).withWidth(gyroscopeWidth * 2).withHeight(mainRowHeight).withMargin(5.0f));
    bottomRow.items.add(juce::FlexItem(rightStick).withWidth(stickWidth).withHeight(mainRowHeight).withMargin(5.0f));
    
    // Add rows to main layout - top row doesn't use flex, others split remaining space
    mainLayout.items.add(juce::FlexItem(topRow).withHeight(shoulderSectionHeight).withMargin(2.0f));  // Fixed height, no flex
    mainLayout.items.add(juce::FlexItem(middleRow).withFlex(1.0f).withMargin(2.0f));
    mainLayout.items.add(juce::FlexItem(bottomRow).withFlex(1.0f).withMargin(2.0f));
    
    // Perform layout
    mainLayout.performLayout(bounds.toFloat());
    
    // Now set the actual bounds for each component based on their FlexItem positions
    for (const auto& item : topRow.items)
    {
        if (auto* comp = dynamic_cast<juce::Component*>(item.associatedComponent))
            comp->setBounds(item.currentBounds.toNearestInt());
    }
    
    for (const auto& item : middleRow.items)
    {
        if (auto* comp = dynamic_cast<juce::Component*>(item.associatedComponent))
            comp->setBounds(item.currentBounds.toNearestInt());
    }
    
    for (const auto& item : bottomRow.items)
    {
        if (auto* comp = dynamic_cast<juce::Component*>(item.associatedComponent))
            comp->setBounds(item.currentBounds.toNearestInt());
    }
}

void ModernGamepadComponent::setupCallbacks()
{
    // ShoulderSection callbacks
    shoulderSection.onButtonClick = [this](const juce::String& button) {
        if (button == "L1") sendMidiCC(MidiCC::L1_BUTTON, 1.0f);
        else if (button == "R1") sendMidiCC(MidiCC::R1_BUTTON, 1.0f);
        else if (button == "L2") sendMidiCC(MidiCC::L2_TRIGGER, 1.0f);
        else if (button == "R2") sendMidiCC(MidiCC::R2_TRIGGER, 1.0f);
    };
    
    shoulderSection.onLearnClick = [this](const juce::String& button) {
        if (button == "L1") sendMidiCC(MidiCC::L1_BUTTON, 1.0f);
        else if (button == "R1") sendMidiCC(MidiCC::R1_BUTTON, 1.0f);
        else if (button == "L2") sendMidiCC(MidiCC::L2_TRIGGER, 1.0f);
        else if (button == "R2") sendMidiCC(MidiCC::R2_TRIGGER, 1.0f);
    };
    
    // D-pad callbacks
    dPad.onButtonClick = [this](const juce::String& button) {
        if (button == "Up") sendMidiCC(MidiCC::DPAD_UP, 1.0f);
        else if (button == "Down") sendMidiCC(MidiCC::DPAD_DOWN, 1.0f);
        else if (button == "Left") sendMidiCC(MidiCC::DPAD_LEFT, 1.0f);
        else if (button == "Right") sendMidiCC(MidiCC::DPAD_RIGHT, 1.0f);
    };
    
    dPad.onLearnClick = [this](const juce::String& button) {
        if (button == "Up") sendMidiCC(MidiCC::DPAD_UP, 1.0f);
        else if (button == "Down") sendMidiCC(MidiCC::DPAD_DOWN, 1.0f);
        else if (button == "Left") sendMidiCC(MidiCC::DPAD_LEFT, 1.0f);
        else if (button == "Right") sendMidiCC(MidiCC::DPAD_RIGHT, 1.0f);
    };
    
    // Face buttons callbacks
    faceButtons.onButtonClick = [this](const juce::String& button) {
        if (button == "A") sendMidiCC(MidiCC::A_BUTTON, 1.0f);
        else if (button == "B") sendMidiCC(MidiCC::B_BUTTON, 1.0f);
        else if (button == "X") sendMidiCC(MidiCC::X_BUTTON, 1.0f);
        else if (button == "Y") sendMidiCC(MidiCC::Y_BUTTON, 1.0f);
    };
    
    faceButtons.onLearnClick = [this](const juce::String& button) {
        if (button == "A") sendMidiCC(MidiCC::A_BUTTON, 1.0f);
        else if (button == "B") sendMidiCC(MidiCC::B_BUTTON, 1.0f);
        else if (button == "X") sendMidiCC(MidiCC::X_BUTTON, 1.0f);
        else if (button == "Y") sendMidiCC(MidiCC::Y_BUTTON, 1.0f);
    };
    
    // Analog sticks callbacks
    leftStick.onAxisChange = [this](const juce::String& axis, float value) {
        if (axis == "X") sendMidiCC(MidiCC::LEFT_STICK_X, (value + 1.0f) * 0.5f);
        else if (axis == "Y") sendMidiCC(MidiCC::LEFT_STICK_Y, (value + 1.0f) * 0.5f);
    };

    leftStick.onLearnClick = [this](const juce::String& control) {
        if (control == "X") sendMidiCC(MidiCC::LEFT_STICK_X, (gamepadState.axes[0] + 1.0f) * 0.5f);
        else if (control == "Y") sendMidiCC(MidiCC::LEFT_STICK_Y, (gamepadState.axes[1] + 1.0f) * 0.5f);
        else if (control == "Press") sendMidiCC(MidiCC::L1_BUTTON, gamepadState.buttons[7] ? 1.0f : 0.0f);
    };

    leftStick.onButtonClick = [this](const juce::String& control) {
        if (control == "X") sendMidiCC(MidiCC::LEFT_STICK_X, (gamepadState.axes[0] + 1.0f) * 0.5f);
        else if (control == "Y") sendMidiCC(MidiCC::LEFT_STICK_Y, (gamepadState.axes[1] + 1.0f) * 0.5f);
        else if (control == "Press") sendMidiCC(MidiCC::L1_BUTTON, gamepadState.buttons[7] ? 1.0f : 0.0f);
    };
    
    rightStick.onAxisChange = [this](const juce::String& axis, float value) {
        if (axis == "X") sendMidiCC(MidiCC::RIGHT_STICK_X, (value + 1.0f) * 0.5f);
        else if (axis == "Y") sendMidiCC(MidiCC::RIGHT_STICK_Y, (value + 1.0f) * 0.5f);
    };

    rightStick.onLearnClick = [this](const juce::String& control) {
        if (control == "X") sendMidiCC(MidiCC::RIGHT_STICK_X, (gamepadState.axes[2] + 1.0f) * 0.5f);
        else if (control == "Y") sendMidiCC(MidiCC::RIGHT_STICK_Y, (gamepadState.axes[3] + 1.0f) * 0.5f);
        else if (control == "Press") sendMidiCC(MidiCC::R1_BUTTON, gamepadState.buttons[8] ? 1.0f : 0.0f);
    };

    rightStick.onButtonClick = [this](const juce::String& control) {
        if (control == "X") sendMidiCC(MidiCC::RIGHT_STICK_X, (gamepadState.axes[2] + 1.0f) * 0.5f);
        else if (control == "Y") sendMidiCC(MidiCC::RIGHT_STICK_Y, (gamepadState.axes[3] + 1.0f) * 0.5f);
        else if (control == "Press") sendMidiCC(MidiCC::R1_BUTTON, gamepadState.buttons[8] ? 1.0f : 0.0f);
    };
    
    // Touchpad callbacks
    touchPad.onXValueChange = [this](float x) {
        sendMidiCC(MidiCC::TOUCHPAD_X, x);
    };
    
    touchPad.onYValueChange = [this](float y) {
        sendMidiCC(MidiCC::TOUCHPAD_Y, y);
    };
    
    touchPad.onPressureValueChange = [this](float pressure) {
        sendMidiCC(MidiCC::TOUCHPAD_PRESSURE, pressure);
    };

    touchPad.onButtonValueChange = [this](float value) {
        sendMidiCC(MidiCC::TOUCHPAD_BUTTON, value);
    };
    
    touchPad.onLearnClick = [this](const juce::String& control) {
        if (control == "X") sendMidiCC(MidiCC::TOUCHPAD_X, 1.0f);
        else if (control == "Y") sendMidiCC(MidiCC::TOUCHPAD_Y, 1.0f);
        else if (control == "Pressure") sendMidiCC(MidiCC::TOUCHPAD_PRESSURE, 1.0f);
        else if (control == "Button") sendMidiCC(MidiCC::TOUCHPAD_BUTTON, 1.0f);
    };
    
    touchPad.onButtonClick = [this](const juce::String& control) {
        if (control == "X") sendMidiCC(MidiCC::TOUCHPAD_X, cachedState.touchpad.x);
        else if (control == "Y") sendMidiCC(MidiCC::TOUCHPAD_Y, cachedState.touchpad.y);
        else if (control == "Pressure") sendMidiCC(MidiCC::TOUCHPAD_PRESSURE, cachedState.touchpad.pressure);
        else if (control == "Button") sendMidiCC(MidiCC::TOUCHPAD_BUTTON, cachedState.touchpad.pressed ? 1.0f : 0.0f);
    };
    
    // Gyroscope callbacks
    gyroscopeDisplay.onValueChange = [this](float x, float y, float z) {
        if (!midiLearnMode) {
            sendMidiCC(MidiCC::GYRO_X, (x + 1.0f) * 0.5f);
            sendMidiCC(MidiCC::GYRO_Y, (y + 1.0f) * 0.5f);
            sendMidiCC(MidiCC::GYRO_Z, (z + 1.0f) * 0.5f);
        }
    };

    gyroscopeDisplay.onLearnClick = [this](const juce::String& axis) {
        if (axis == "X") sendMidiCC(MidiCC::GYRO_X, (gamepadState.gyroscope.x + 1.0f) * 0.5f);
        else if (axis == "Y") sendMidiCC(MidiCC::GYRO_Y, (gamepadState.gyroscope.y + 1.0f) * 0.5f);
        else if (axis == "Z") sendMidiCC(MidiCC::GYRO_Z, (gamepadState.gyroscope.z + 1.0f) * 0.5f);
    };

    gyroscopeDisplay.onButtonClick = [this](const juce::String& axis) {
        if (!midiLearnMode) {
            if (axis == "X") sendMidiCC(MidiCC::GYRO_X, (gamepadState.gyroscope.x + 1.0f) * 0.5f);
            else if (axis == "Y") sendMidiCC(MidiCC::GYRO_Y, (gamepadState.gyroscope.y + 1.0f) * 0.5f);
            else if (axis == "Z") sendMidiCC(MidiCC::GYRO_Z, (gamepadState.gyroscope.z + 1.0f) * 0.5f);
        }
    };

    // Accelerometer callbacks
    accelerometerDisplay.onValueChange = [this](float x, float y, float z) {
        if (!midiLearnMode) {
            sendMidiCC(MidiCC::ACCEL_X, (x + 1.0f) * 0.5f);
            sendMidiCC(MidiCC::ACCEL_Y, (y + 1.0f) * 0.5f);
            sendMidiCC(MidiCC::ACCEL_Z, (z + 1.0f) * 0.5f);
        }
    };

    accelerometerDisplay.onLearnClick = [this](const juce::String& axis) {
        if (axis == "X") sendMidiCC(MidiCC::ACCEL_X, (gamepadState.accelerometer.x + 1.0f) * 0.5f);
        else if (axis == "Y") sendMidiCC(MidiCC::ACCEL_Y, (gamepadState.accelerometer.y + 1.0f) * 0.5f);
        else if (axis == "Z") sendMidiCC(MidiCC::ACCEL_Z, (gamepadState.accelerometer.z + 1.0f) * 0.5f);
    };

    accelerometerDisplay.onButtonClick = [this](const juce::String& axis) {
        if (!midiLearnMode) {
            if (axis == "X") sendMidiCC(MidiCC::ACCEL_X, (gamepadState.accelerometer.x + 1.0f) * 0.5f);
            else if (axis == "Y") sendMidiCC(MidiCC::ACCEL_Y, (gamepadState.accelerometer.y + 1.0f) * 0.5f);
            else if (axis == "Z") sendMidiCC(MidiCC::ACCEL_Z, (gamepadState.accelerometer.z + 1.0f) * 0.5f);
        }
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
    // Use the already normalized values from GamepadManager
    float l2Value = newState.axes[4];
    float r2Value = newState.axes[5];

    // Update shoulder section
    shoulderSection.setState({
        newState.buttons[9],  // L1
        newState.buttons[10], // R1
        l2Value,  // L2
        r2Value,  // R2
        midiLearnMode,
        MidiCC::L1_BUTTON,
        MidiCC::R1_BUTTON,
        MidiCC::L2_TRIGGER,
        MidiCC::R2_TRIGGER
    });
    
    // Update D-pad
    dPad.setState({
        newState.buttons[11], // Up
        newState.buttons[12], // Down
        newState.buttons[13], // Left
        newState.buttons[14], // Right
        MidiCC::DPAD_UP,
        MidiCC::DPAD_DOWN,
        MidiCC::DPAD_LEFT,
        MidiCC::DPAD_RIGHT,
        midiLearnMode
    });
    
    // Update face buttons
    faceButtons.setState({
        MidiCC::A_BUTTON,
        MidiCC::B_BUTTON,
        MidiCC::X_BUTTON,
        MidiCC::Y_BUTTON,
        newState.buttons[0],  // A
        newState.buttons[1],  // B
        newState.buttons[2],  // X
        newState.buttons[3],  // Y
        midiLearnMode
    });

    // Update analog sticks
    {
        AnalogStick::State stickState;
        stickState.isEnabled = true;
        stickState.xValue = juce::jlimit(-1.0f, 1.0f, newState.axes[0]);
        stickState.yValue = juce::jlimit(-1.0f, 1.0f, newState.axes[1]);
        stickState.isPressed = newState.buttons[7];
        stickState.xCC = MidiCC::LEFT_STICK_X;
        stickState.yCC = MidiCC::LEFT_STICK_Y;
        stickState.pressCC = MidiCC::L1_BUTTON;
        stickState.isLearnMode = midiLearnMode;
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
        stickState.xCC = MidiCC::RIGHT_STICK_X;
        stickState.yCC = MidiCC::RIGHT_STICK_Y;
        stickState.pressCC = MidiCC::R1_BUTTON;
        stickState.isLearnMode = midiLearnMode;
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
        padState.touched = newState.touchpad.touched;
        padState.xCC = MidiCC::TOUCHPAD_X;
        padState.yCC = MidiCC::TOUCHPAD_Y;
        padState.pressureCC = MidiCC::TOUCHPAD_PRESSURE;
        padState.buttonCC = MidiCC::TOUCHPAD_BUTTON;
        padState.isLearnMode = midiLearnMode;
        touchPad.setState(padState);
    }

    // Update gyroscope
    {
        SensorDisplay::State gyroState;
        gyroState.enabled = newState.gyroscope.enabled;
        gyroState.x = juce::jlimit(-1.0f, 1.0f, newState.gyroscope.x);
        gyroState.y = juce::jlimit(-1.0f, 1.0f, newState.gyroscope.y);
        gyroState.z = juce::jlimit(-1.0f, 1.0f, newState.gyroscope.z);
        gyroState.xCC = MidiCC::GYRO_X;
        gyroState.yCC = MidiCC::GYRO_Y;
        gyroState.zCC = MidiCC::GYRO_Z;
        gyroState.isLearnMode = midiLearnMode;
        gyroState.isAccelerometer = false;
        gyroscopeDisplay.setState(gyroState);
    }

    // Update accelerometer
    {
        SensorDisplay::State accelState;
        accelState.enabled = newState.accelerometer.enabled;
        accelState.x = juce::jlimit(-1.0f, 1.0f, newState.accelerometer.x);
        accelState.y = juce::jlimit(-1.0f, 1.0f, newState.accelerometer.y);
        accelState.z = juce::jlimit(-1.0f, 1.0f, newState.accelerometer.z);
        accelState.xCC = MidiCC::ACCEL_X;
        accelState.yCC = MidiCC::ACCEL_Y;
        accelState.zCC = MidiCC::ACCEL_Z;
        accelState.isLearnMode = midiLearnMode;
        accelState.isAccelerometer = true;
        accelerometerDisplay.setState(accelState);
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
        gyroscopeDisplay.setLearnMode(enabled);
        accelerometerDisplay.setLearnMode(enabled);
        
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