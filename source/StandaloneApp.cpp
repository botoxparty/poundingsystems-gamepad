#include "StandaloneApp.h"

StandaloneApp::StandaloneApp()
{
    // Initialize MIDI output manager early
    MidiOutputManager::getInstance();
    
    // Create single gamepad component
    gamepadComponent = std::make_unique<ModernGamepadComponent>(gamepadManager.getGamepadState(0));
    addAndMakeVisible(gamepadComponent.get());  // Make visible immediately
    
    // Set up logo
    auto logoImage = juce::ImageCache::getFromMemory(BinaryData::PoundingSystemsLogo_png, BinaryData::PoundingSystemsLogo_pngSize);
    logoComponent.setImage(logoImage);
    logoComponent.setImagePlacement(juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize);
    logoComponent.setMouseCursor(juce::MouseCursor::PointingHandCursor);
    logoComponent.addMouseListener(this, false);
    
    // Add components
    addAndMakeVisible(logoComponent);
    
    // Set up MIDI mappings
    setupMidiMappings();
    
    // Add gamepad state change callback
    gamepadManager.addStateChangeCallback([this] { handleGamepadStateChange(); });
    
    // Start timer to update UI (30fps)
    startTimer(33);
    
    setSize(800, 600);
}

StandaloneApp::~StandaloneApp()
{
    stopTimer();
}

void StandaloneApp::paint(juce::Graphics& g)
{
    // Medium gray background (#C0C0C0)
    g.fillAll(juce::Colour(192, 192, 192));
    
    // Simple border
    g.setColour(juce::Colours::darkgrey);
    g.drawRect(getLocalBounds().toFloat(), 1.0f);
}

void StandaloneApp::resized()
{
    auto area = getLocalBounds();
    
    // Footer with logo
    auto logoHeight = 25;
    auto footerArea = area.removeFromBottom(logoHeight);
    auto logoArea = footerArea.reduced(5, 0);
    logoComponent.setBounds(logoArea);
    
    // Gamepad area with padding
    auto gamepadArea = area.reduced(5, 5);
    gamepadComponent->setBounds(gamepadArea);
}

void StandaloneApp::timerCallback()
{
    // Always update the gamepad component
    gamepadComponent->updateState(gamepadManager.getGamepadState(0));
}

void StandaloneApp::handleGamepadStateChange()
{
    const auto& currentState = gamepadManager.getGamepadState(0);
    auto& previousState = previousGamepadState;

    // Check if connection state changed
    bool connectionStateChanged = (currentState.connected != previousState.connected);

    if (currentState.connected)
    {
        // Process axes - only send if changed
        for (int axisIndex = 0; axisIndex < GamepadManager::MAX_AXES; ++axisIndex)
        {
            if (currentState.axes[axisIndex] != previousState.axes[axisIndex])
            {
                const auto& mapping = axisMappings[axisIndex];
                float normalizedValue = (currentState.axes[axisIndex] + 1.0f) * 0.5f;
                int midiValue = juce::jmap(normalizedValue, 0.0f, 1.0f, 
                                         static_cast<float>(mapping.minValue), 
                                         static_cast<float>(mapping.maxValue));
                
                MidiOutputManager::getInstance().sendControlChange(mapping.channel, mapping.ccNumber, midiValue);
                
                // Update previous state
                previousState.axes[axisIndex] = currentState.axes[axisIndex];
            }
        }
        
        // Process buttons - only send if changed
        for (int buttonIndex = 0; buttonIndex < GamepadManager::MAX_BUTTONS; ++buttonIndex)
        {
            if (currentState.buttons[buttonIndex] != previousState.buttons[buttonIndex])
            {
                const auto& mapping = buttonMappings[buttonIndex];
                int midiValue = currentState.buttons[buttonIndex] ? mapping.maxValue : mapping.minValue;
                
                MidiOutputManager::getInstance().sendControlChange(mapping.channel, mapping.ccNumber, midiValue);
                
                // Update previous state
                previousState.buttons[buttonIndex] = currentState.buttons[buttonIndex];
            }
        }

        // Process touchpad state - only send if touched and values changed
        if (currentState.touchpad.touched)
        {
            // Update touched state first
            if (!previousState.touchpad.touched)
            {
                previousState.touchpad.touched = true;
            }

            // Send X position if changed
            if (currentState.touchpad.x != previousState.touchpad.x)
            {
                MidiOutputManager::getInstance().sendControlChange(1, MidiCC::TOUCHPAD_X, 
                    static_cast<int>(currentState.touchpad.x * 127.0f));
                previousState.touchpad.x = currentState.touchpad.x;
            }

            // Send Y position if changed
            if (currentState.touchpad.y != previousState.touchpad.y)
            {
                MidiOutputManager::getInstance().sendControlChange(1, MidiCC::TOUCHPAD_Y, 
                    static_cast<int>(currentState.touchpad.y * 127.0f));
                previousState.touchpad.y = currentState.touchpad.y;
            }

            // Send pressure if changed
            if (currentState.touchpad.pressure != previousState.touchpad.pressure)
            {
                MidiOutputManager::getInstance().sendControlChange(1, MidiCC::TOUCHPAD_PRESSURE, 
                    static_cast<int>(currentState.touchpad.pressure * 127.0f));
                previousState.touchpad.pressure = currentState.touchpad.pressure;
            }
        }
        else if (previousState.touchpad.touched)
        {
            // If touchpad was touched before but not now, send zero values
            MidiOutputManager::getInstance().sendControlChange(1, MidiCC::TOUCHPAD_X, 0);
            MidiOutputManager::getInstance().sendControlChange(1, MidiCC::TOUCHPAD_Y, 0);
            MidiOutputManager::getInstance().sendControlChange(1, MidiCC::TOUCHPAD_PRESSURE, 0);
            previousState.touchpad.touched = false;
            previousState.touchpad.x = 0.0f;
            previousState.touchpad.y = 0.0f;
            previousState.touchpad.pressure = 0.0f;
        }

        // Send touchpad button state if changed
        if (currentState.touchpad.pressed != previousState.touchpad.pressed)
        {
            MidiOutputManager::getInstance().sendControlChange(1, MidiCC::TOUCHPAD_BUTTON, 
                currentState.touchpad.pressed ? 127 : 0);
            previousState.touchpad.pressed = currentState.touchpad.pressed;
        }

        // Process gyroscope state if enabled and not in learn mode
        if (currentState.gyroscope.enabled && !gamepadComponent->isMidiLearnMode())
        {
            // Send X axis if changed
            if (currentState.gyroscope.x != previousState.gyroscope.x)
            {
                float normalizedX = (currentState.gyroscope.x + 10.0f) * 0.5f * 127.0f / 10.0f;
                MidiOutputManager::getInstance().sendControlChange(1, MidiCC::GYRO_X, 
                    static_cast<int>(juce::jlimit(0.0f, 127.0f, normalizedX)));
                previousState.gyroscope.x = currentState.gyroscope.x;
            }

            // Send Y axis if changed
            if (currentState.gyroscope.y != previousState.gyroscope.y)
            {
                float normalizedY = (currentState.gyroscope.y + 10.0f) * 0.5f * 127.0f / 10.0f;
                MidiOutputManager::getInstance().sendControlChange(1, MidiCC::GYRO_Y, 
                    static_cast<int>(juce::jlimit(0.0f, 127.0f, normalizedY)));
                previousState.gyroscope.y = currentState.gyroscope.y;
            }

            // Send Z axis if changed
            if (currentState.gyroscope.z != previousState.gyroscope.z)
            {
                float normalizedZ = (currentState.gyroscope.z + 10.0f) * 0.5f * 127.0f / 10.0f;
                MidiOutputManager::getInstance().sendControlChange(1, MidiCC::GYRO_Z, 
                    static_cast<int>(juce::jlimit(0.0f, 127.0f, normalizedZ)));
                previousState.gyroscope.z = currentState.gyroscope.z;
            }
        }
    }
    
    // Update connection state
    previousState.connected = currentState.connected;

    // If connection state changed, trigger a layout update
    if (connectionStateChanged)
    {
        resized();
    }
}

void StandaloneApp::setupMidiMappings()
{
    // Map axes to CC numbers from MidiCC mapping
    axisMappings[0] = { 1, MidiCC::LEFT_STICK_X, 0, 127, false }; // Left stick X
    axisMappings[1] = { 1, MidiCC::LEFT_STICK_Y, 0, 127, false }; // Left stick Y
    axisMappings[2] = { 1, MidiCC::RIGHT_STICK_X, 0, 127, false }; // Right stick X
    axisMappings[3] = { 1, MidiCC::RIGHT_STICK_Y, 0, 127, false }; // Right stick Y
    axisMappings[4] = { 1, MidiCC::L2_TRIGGER, 0, 127, false }; // L2 trigger
    axisMappings[5] = { 1, MidiCC::R2_TRIGGER, 0, 127, false }; // R2 trigger

    // Map buttons to CC numbers from MidiCC mapping
    buttonMappings[0] = { 1, MidiCC::A_BUTTON, 0, 127, true }; // A button
    buttonMappings[1] = { 1, MidiCC::B_BUTTON, 0, 127, true }; // B button
    buttonMappings[2] = { 1, MidiCC::X_BUTTON, 0, 127, true }; // X button
    buttonMappings[3] = { 1, MidiCC::Y_BUTTON, 0, 127, true }; // Y button
    
    // Shoulder buttons
    buttonMappings[9] = { 1, MidiCC::L1_BUTTON, 0, 127, true }; // L1 button
    buttonMappings[10] = { 1, MidiCC::R1_BUTTON, 0, 127, true }; // R1 button

    // D-pad
    buttonMappings[11] = { 1, MidiCC::DPAD_UP, 0, 127, true }; // D-pad Up
    buttonMappings[12] = { 1, MidiCC::DPAD_DOWN, 0, 127, true }; // D-pad Down
    buttonMappings[13] = { 1, MidiCC::DPAD_LEFT, 0, 127, true }; // D-pad Left
    buttonMappings[14] = { 1, MidiCC::DPAD_RIGHT, 0, 127, true }; // D-pad Right

    // Initialize remaining buttons with sequential CC numbers starting after the mapped ones
    for (int buttonIndex = 0; buttonIndex < GamepadManager::MAX_BUTTONS; ++buttonIndex)
    {
        if (buttonMappings[buttonIndex].ccNumber == 0) // If not already mapped
        {
            buttonMappings[buttonIndex] = {
                1, // channel
                42 + buttonIndex, // CC number (starting after our mapped range)
                0, // min value
                127, // max value
                true // is a button
            };
        }
    }
}

void StandaloneApp::handleLogoClick()
{
    auto* aboutWindow = new AboutWindow();
    aboutWindow->enterModalState(true, nullptr, true);
}

void StandaloneApp::mouseUp(const juce::MouseEvent& event)
{
    if (event.eventComponent == &logoComponent)
    {
        handleLogoClick();
    }
}
