#include "ModernGamepadComponent.h"
#include "MidiCCMapping.h"
#include "../StandaloneApp.h"

ModernGamepadComponent::ModernGamepadComponent(const GamepadManager::GamepadState& state, StandaloneApp& app)
    : gamepadState(state)
    , app(app)
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
    learnModeButton.setButtonText("Teach Mode");
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
        if (button == "L1") sendMidiCC(9, 1.0f, true);  // L1 is button 9
        else if (button == "R1") sendMidiCC(10, 1.0f, true);  // R1 is button 10
        else if (button == "L2") sendMidiCC(4, 1.0f, false);  // L2 is axis 4
        else if (button == "R2") sendMidiCC(5, 1.0f, false);  // R2 is axis 5
    };
    
    shoulderSection.onLearnClick = [this](const juce::String& button) {
        if (button == "L1") sendMidiCC(9, 1.0f, true);
        else if (button == "R1") sendMidiCC(10, 1.0f, true);
        else if (button == "L2") sendMidiCC(4, 1.0f, false);
        else if (button == "R2") sendMidiCC(5, 1.0f, false);
    };
    
    // D-pad callbacks
    dPad.onButtonClick = [this](const juce::String& button) {
        if (button == "Up") sendMidiCC(11, 1.0f, true);  // D-pad Up is button 11
        else if (button == "Down") sendMidiCC(12, 1.0f, true);  // D-pad Down is button 12
        else if (button == "Left") sendMidiCC(13, 1.0f, true);  // D-pad Left is button 13
        else if (button == "Right") sendMidiCC(14, 1.0f, true);  // D-pad Right is button 14
    };
    
    dPad.onLearnClick = [this](const juce::String& button) {
        if (button == "Up") sendMidiCC(11, 1.0f, true);
        else if (button == "Down") sendMidiCC(12, 1.0f, true);
        else if (button == "Left") sendMidiCC(13, 1.0f, true);
        else if (button == "Right") sendMidiCC(14, 1.0f, true);
    };
    
    // Face buttons callbacks
    faceButtons.onButtonClick = [this](const juce::String& button) {
        if (button == "A") sendMidiCC(0, 1.0f, true);  // A is button 0
        else if (button == "B") sendMidiCC(1, 1.0f, true);  // B is button 1
        else if (button == "X") sendMidiCC(2, 1.0f, true);  // X is button 2
        else if (button == "Y") sendMidiCC(3, 1.0f, true);  // Y is button 3
    };
    
    faceButtons.onLearnClick = [this](const juce::String& button) {
        if (button == "A") sendMidiCC(0, 1.0f, true);
        else if (button == "B") sendMidiCC(1, 1.0f, true);
        else if (button == "X") sendMidiCC(2, 1.0f, true);
        else if (button == "Y") sendMidiCC(3, 1.0f, true);
    };
    
    // Analog sticks callbacks
    leftStick.onAxisChange = [this](const juce::String& axis, float value) {
        if (axis == "X") sendMidiCC(0, (value + 1.0f) * 0.5f, false);  // Left stick X is axis 0
        else if (axis == "Y") sendMidiCC(1, (value + 1.0f) * 0.5f, false);  // Left stick Y is axis 1
    };

    leftStick.onLearnClick = [this](const juce::String& control) {
        if (control == "X") sendMidiCC(0, (gamepadState.axes[0] + 1.0f) * 0.5f, false);
        else if (control == "Y") sendMidiCC(1, (gamepadState.axes[1] + 1.0f) * 0.5f, false);
        else if (control == "Press") sendMidiCC(9, gamepadState.buttons[7] ? 1.0f : 0.0f, true);
    };

    leftStick.onButtonClick = [this](const juce::String& control) {
        if (control == "X") sendMidiCC(0, (gamepadState.axes[0] + 1.0f) * 0.5f, false);
        else if (control == "Y") sendMidiCC(1, (gamepadState.axes[1] + 1.0f) * 0.5f, false);
        else if (control == "Press") sendMidiCC(9, gamepadState.buttons[7] ? 1.0f : 0.0f, true);
    };
    
    rightStick.onAxisChange = [this](const juce::String& axis, float value) {
        if (axis == "X") sendMidiCC(2, (value + 1.0f) * 0.5f, false);  // Right stick X is axis 2
        else if (axis == "Y") sendMidiCC(3, (value + 1.0f) * 0.5f, false);  // Right stick Y is axis 3
    };

    rightStick.onLearnClick = [this](const juce::String& control) {
        if (control == "X") sendMidiCC(2, (gamepadState.axes[2] + 1.0f) * 0.5f, false);
        else if (control == "Y") sendMidiCC(3, (gamepadState.axes[3] + 1.0f) * 0.5f, false);
        else if (control == "Press") sendMidiCC(10, gamepadState.buttons[8] ? 1.0f : 0.0f, true);
    };

    rightStick.onButtonClick = [this](const juce::String& control) {
        if (control == "X") sendMidiCC(2, (gamepadState.axes[2] + 1.0f) * 0.5f, false);
        else if (control == "Y") sendMidiCC(3, (gamepadState.axes[3] + 1.0f) * 0.5f, false);
        else if (control == "Press") sendMidiCC(10, gamepadState.buttons[8] ? 1.0f : 0.0f, true);
    };
    
    // Touchpad callbacks
    touchPad.onXValueChange = [this](float x) {
        sendMidiCC(6, (x + 1.0f) * 0.5f, false);  // Touchpad X is axis 6
    };
    
    touchPad.onYValueChange = [this](float y) {
        sendMidiCC(7, (y + 1.0f) * 0.5f, false);  // Touchpad Y is axis 7
    };
    
    touchPad.onPressureValueChange = [this](float pressure) {
        sendMidiCC(8, pressure, false);  // Touchpad pressure is axis 8
    };

    touchPad.onButtonValueChange = [this](float value) {
        sendMidiCC(15, value, true);  // Touchpad button is button 15
    };
    
    touchPad.onLearnClick = [this](const juce::String& control) {
        if (control == "X") sendMidiCC(6, 1.0f, false);
        else if (control == "Y") sendMidiCC(7, 1.0f, false);
        else if (control == "Pressure") sendMidiCC(8, 1.0f, false);
        else if (control == "Button") sendMidiCC(15, 1.0f, true);
    };
    
    touchPad.onButtonClick = [this](const juce::String& control) {
        if (control == "X") sendMidiCC(6, cachedState.touchpad.x, false);
        else if (control == "Y") sendMidiCC(7, cachedState.touchpad.y, false);
        else if (control == "Pressure") sendMidiCC(8, cachedState.touchpad.pressure, false);
        else if (control == "Button") sendMidiCC(15, cachedState.touchpad.pressed ? 1.0f : 0.0f, true);
    };
    
    // Gyroscope callbacks

    gyroscopeDisplay.onLearnClick = [this](const juce::String& axis) {
        if (axis == "X") sendMidiCC(0, (gamepadState.gyroscope.x + 1.0f) * 0.5f, false);
        else if (axis == "Y") sendMidiCC(1, (gamepadState.gyroscope.y + 1.0f) * 0.5f, false);
        else if (axis == "Z") sendMidiCC(2, (gamepadState.gyroscope.z + 1.0f) * 0.5f, false);
    };

    gyroscopeDisplay.onButtonClick = [this](const juce::String& axis) {
        if (!midiLearnMode) {
            if (axis == "X") sendMidiCC(0, (gamepadState.gyroscope.x + 1.0f) * 0.5f, false);
            else if (axis == "Y") sendMidiCC(1, (gamepadState.gyroscope.y + 1.0f) * 0.5f, false);
            else if (axis == "Z") sendMidiCC(2, (gamepadState.gyroscope.z + 1.0f) * 0.5f, false);
        }
    };


    accelerometerDisplay.onLearnClick = [this](const juce::String& axis) {
        if (axis == "X") sendMidiCC(3, (gamepadState.accelerometer.x + 1.0f) * 0.5f, false);
        else if (axis == "Y") sendMidiCC(4, (gamepadState.accelerometer.y + 1.0f) * 0.5f, false);
        else if (axis == "Z") sendMidiCC(5, (gamepadState.accelerometer.z + 1.0f) * 0.5f, false);
    };

    accelerometerDisplay.onButtonClick = [this](const juce::String& axis) {
        if (!midiLearnMode) {
            if (axis == "X") sendMidiCC(3, (gamepadState.accelerometer.x + 1.0f) * 0.5f, false);
            else if (axis == "Y") sendMidiCC(4, (gamepadState.accelerometer.y + 1.0f) * 0.5f, false);
            else if (axis == "Z") sendMidiCC(5, (gamepadState.accelerometer.z + 1.0f) * 0.5f, false);
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
        app.buttonMappings[9].empty() ? 0 : app.buttonMappings[9][0].ccNumber,  // L1
        app.buttonMappings[10].empty() ? 0 : app.buttonMappings[10][0].ccNumber,  // R1
        app.axisMappings[4].empty() ? 0 : app.axisMappings[4][0].ccNumber,  // L2
        app.axisMappings[5].empty() ? 0 : app.axisMappings[5][0].ccNumber   // R2
    });
    
    // Update D-pad
    dPad.setState({
        newState.buttons[11], // Up
        newState.buttons[12], // Down
        newState.buttons[13], // Left
        newState.buttons[14], // Right
        app.buttonMappings[11].empty() ? 0 : app.buttonMappings[11][0].ccNumber,  // Up
        app.buttonMappings[12].empty() ? 0 : app.buttonMappings[12][0].ccNumber,  // Down
        app.buttonMappings[13].empty() ? 0 : app.buttonMappings[13][0].ccNumber,  // Left
        app.buttonMappings[14].empty() ? 0 : app.buttonMappings[14][0].ccNumber,  // Right
        midiLearnMode
    });
    
    // Update face buttons
    faceButtons.setState({
        app.buttonMappings[0].empty() ? 0 : app.buttonMappings[0][0].ccNumber,  // A
        app.buttonMappings[1].empty() ? 0 : app.buttonMappings[1][0].ccNumber,  // B
        app.buttonMappings[2].empty() ? 0 : app.buttonMappings[2][0].ccNumber,  // X
        app.buttonMappings[3].empty() ? 0 : app.buttonMappings[3][0].ccNumber,  // Y
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
        stickState.xCC = app.axisMappings[0].empty() ? 0 : app.axisMappings[0][0].ccNumber;  // Left X
        stickState.yCC = app.axisMappings[1].empty() ? 0 : app.axisMappings[1][0].ccNumber;  // Left Y
        stickState.pressCC = app.buttonMappings[7].empty() ? 0 : app.buttonMappings[7][0].ccNumber;  // Left stick press
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
        stickState.xCC = app.axisMappings[2].empty() ? 0 : app.axisMappings[2][0].ccNumber;  // Right X
        stickState.yCC = app.axisMappings[3].empty() ? 0 : app.axisMappings[3][0].ccNumber;  // Right Y
        stickState.pressCC = app.buttonMappings[8].empty() ? 0 : app.buttonMappings[8][0].ccNumber;  // Right stick press
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
        padState.xCC = app.axisMappings[6].empty() ? 0 : app.axisMappings[6][0].ccNumber;  // Touchpad X
        padState.yCC = app.axisMappings[7].empty() ? 0 : app.axisMappings[7][0].ccNumber;  // Touchpad Y
        padState.pressureCC = app.axisMappings[8].empty() ? 0 : app.axisMappings[8][0].ccNumber;  // Touchpad pressure
        padState.buttonCC = app.buttonMappings[15].empty() ? 0 : app.buttonMappings[15][0].ccNumber;  // Touchpad button
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
        gyroState.xCC = app.gyroMappings[0].empty() ? 0 : app.gyroMappings[0][0].ccNumber;  // Gyro X
        gyroState.yCC = app.gyroMappings[1].empty() ? 0 : app.gyroMappings[1][0].ccNumber;  // Gyro Y
        gyroState.zCC = app.gyroMappings[2].empty() ? 0 : app.gyroMappings[2][0].ccNumber;  // Gyro Z
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
        accelState.xCC = app.accelerometerMappings[0].empty() ? 0 : app.accelerometerMappings[0][0].ccNumber;  // Accel X
        accelState.yCC = app.accelerometerMappings[1].empty() ? 0 : app.accelerometerMappings[1][0].ccNumber;  // Accel Y
        accelState.zCC = app.accelerometerMappings[2].empty() ? 0 : app.accelerometerMappings[2][0].ccNumber;  // Accel Z
        accelState.isLearnMode = midiLearnMode;
        accelState.isAccelerometer = true;
        accelerometerDisplay.setState(accelState);
    }

    // Update status label
    statusLabel.setText(newState.connected
        ? "Connected: " + newState.name + (midiLearnMode ? " (Teach Mode)" : "")
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

void ModernGamepadComponent::sendMidiCC(int controlIndex, float value, bool isButton)
{
    if (isButton)
    {
        // Get button mappings
        const auto& mappings = app.buttonMappings[static_cast<size_t>(controlIndex)];
        for (const auto& mapping : mappings)
        {
            float mappedValue = value * (mapping.maxValue - mapping.minValue) + mapping.minValue;
            
            if (mapping.type == StandaloneApp::MidiMapping::Type::ControlChange)
            {
                MidiOutputManager::getInstance().sendControlChange(mapping.channel, mapping.ccNumber, static_cast<int>(mappedValue));
            }
            else // Note
            {
                // For buttons, we send note on when pressed and note off when released
                if (value > 0.5f)
                {
                    MidiOutputManager::getInstance().sendNoteOn(mapping.channel, mapping.noteNumber, mappedValue / 127.0f);
                }
                else
                {
                    // Send note off with zero velocity
                    MidiOutputManager::getInstance().sendNoteOn(mapping.channel, mapping.noteNumber, 0.0f);
                }
            }
        }
    }
    else
    {
        // Get axis mappings
        const auto& mappings = app.axisMappings[static_cast<size_t>(controlIndex)];
        for (const auto& mapping : mappings)
        {
            float mappedValue = value * (mapping.maxValue - mapping.minValue) + mapping.minValue;
            
            if (mapping.type == StandaloneApp::MidiMapping::Type::ControlChange)
            {
                MidiOutputManager::getInstance().sendControlChange(mapping.channel, mapping.ccNumber, static_cast<int>(mappedValue));
            }
            else // Note
            {
                // For axes, we send note on with velocity based on the axis value
                // We only send note on when the value is above a certain threshold
                if (mappedValue > 0)
                {
                    MidiOutputManager::getInstance().sendNoteOn(mapping.channel, mapping.noteNumber, mappedValue / 127.0f);
                }
                else
                {
                    // Send note off with zero velocity
                    MidiOutputManager::getInstance().sendNoteOn(mapping.channel, mapping.noteNumber, 0.0f);
                }
            }
        }
    }
}

void ModernGamepadComponent::timerCallback()
{
    updateState(gamepadState);
} 