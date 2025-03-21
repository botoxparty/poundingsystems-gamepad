#include "StandaloneApp.h"

StandaloneApp::StandaloneApp()
{
    // Set up title label
    titleLabel.setText("Gamepad MIDI Controller", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(24.0f, juce::Font::bold));
    titleLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(titleLabel);
    
    // Set up status label
    statusLabel.setText("No gamepads connected", juce::dontSendNotification);
    statusLabel.setFont(juce::Font(16.0f));
    statusLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(statusLabel);
    
    // Set up MIDI device selector
    midiDeviceSelector.setTextWhenNothingSelected("Select MIDI Output Device");
    addAndMakeVisible(midiDeviceSelector);
    refreshMidiDevices();
    
    midiDeviceSelector.onChange = [this] { midiDeviceChanged(); };
    
    // Create gamepad components
    for (int i = 0; i < GamepadManager::MAX_GAMEPADS; ++i)
    {
        gamepadComponents[i] = std::make_unique<GamepadComponent>(
            gamepadManager.getGamepadState(i));
        addChildComponent(gamepadComponents[i].get());
    }
    
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
    g.fillAll(juce::Colour(30, 30, 30));
}

void StandaloneApp::resized()
{
    auto area = getLocalBounds().reduced(20);
    
    // Position title at the top
    titleLabel.setBounds(area.removeFromTop(40));
    
    // Position status and MIDI device selector
    auto topArea = area.removeFromTop(60);
    statusLabel.setBounds(topArea.removeFromTop(30));
    midiDeviceSelector.setBounds(topArea.withSizeKeepingCentre(300, 25));
    
    // Position gamepad components
    int connectedGamepads = gamepadManager.getNumConnectedGamepads();
    
    if (connectedGamepads > 0)
    {
        int heightPerGamepad = area.getHeight() / connectedGamepads;
        
        int visibleCount = 0;
        for (int i = 0; i < GamepadManager::MAX_GAMEPADS; ++i)
        {
            if (gamepadManager.isGamepadConnected(i))
            {
                auto componentBounds = area.removeFromTop(heightPerGamepad).reduced(10);
                gamepadComponents[i]->setBounds(componentBounds);
                gamepadComponents[i]->setVisible(true);
                visibleCount++;
            }
            else
            {
                gamepadComponents[i]->setVisible(false);
            }
        }
    }
}

void StandaloneApp::timerCallback()
{
    // Update gamepad components
    for (int i = 0; i < GamepadManager::MAX_GAMEPADS; ++i)
    {
        if (gamepadComponents[i]->isVisible())
        {
            gamepadComponents[i]->updateState(gamepadManager.getGamepadState(i));
        }
    }
    
    // Update status label
    int numGamepads = gamepadManager.getNumConnectedGamepads();
    statusLabel.setText(numGamepads > 0 
        ? juce::String(numGamepads) + " gamepad" + (numGamepads > 1 ? "s" : "") + " connected"
        : "No gamepads connected",
        juce::dontSendNotification);
}

void StandaloneApp::handleGamepadStateChange()
{
    // Process each gamepad's state
    for (int gamepadIndex = 0; gamepadIndex < GamepadManager::MAX_GAMEPADS; ++gamepadIndex)
    {
        const auto& currentState = gamepadManager.getGamepadState(gamepadIndex);
        auto& previousState = previousGamepadStates[gamepadIndex];

        if (currentState.connected)
        {
            // Process axes - only send if changed
            for (int axisIndex = 0; axisIndex < GamepadManager::MAX_AXES; ++axisIndex)
            {
                if (currentState.axes[axisIndex] != previousState.axes[axisIndex])
                {
                    const auto& mapping = axisMappings[gamepadIndex][axisIndex];
                    float normalizedValue = (currentState.axes[axisIndex] + 1.0f) * 0.5f;
                    int midiValue = juce::jmap(normalizedValue, 0.0f, 1.0f, 
                                             static_cast<float>(mapping.minValue), 
                                             static_cast<float>(mapping.maxValue));
                    
                    midiOutput.sendControlChange(mapping.channel, mapping.ccNumber, midiValue);
                    
                    // Update previous state
                    previousState.axes[axisIndex] = currentState.axes[axisIndex];
                }
            }
            
            // Process buttons - only send if changed
            for (int buttonIndex = 0; buttonIndex < GamepadManager::MAX_BUTTONS; ++buttonIndex)
            {
                if (currentState.buttons[buttonIndex] != previousState.buttons[buttonIndex])
                {
                    const auto& mapping = buttonMappings[gamepadIndex][buttonIndex];
                    int midiValue = currentState.buttons[buttonIndex] ? mapping.maxValue : mapping.minValue;
                    
                    midiOutput.sendControlChange(mapping.channel, mapping.ccNumber, midiValue);
                    
                    // Update previous state
                    previousState.buttons[buttonIndex] = currentState.buttons[buttonIndex];
                }
            }
        }
        
        // Update connection state
        previousState.connected = currentState.connected;
    }
}

void StandaloneApp::setupMidiMappings()
{
    // Set up default MIDI mappings for each gamepad
    for (int gamepadIndex = 0; gamepadIndex < GamepadManager::MAX_GAMEPADS; ++gamepadIndex)
    {
        // Map axes to CC 1-6 on channel 1 (JUCE uses 1-16 for channels)
        for (int axisIndex = 0; axisIndex < GamepadManager::MAX_AXES; ++axisIndex)
        {
            axisMappings[gamepadIndex][axisIndex] = {
                1, // channel (1-based for JUCE)
                axisIndex + 1, // CC number
                0, // min value
                127, // max value
                false // not a button
            };
        }
        
        // Map buttons to CC 20-34 on channel 1 (JUCE uses 1-16 for channels)
        for (int buttonIndex = 0; buttonIndex < GamepadManager::MAX_BUTTONS; ++buttonIndex)
        {
            buttonMappings[gamepadIndex][buttonIndex] = {
                1, // channel (1-based for JUCE)
                buttonIndex + 20, // CC number
                0, // min value
                127, // max value
                true // is a button
            };
        }
    }
}

void StandaloneApp::refreshMidiDevices()
{
    midiDeviceSelector.clear();
    
    auto devices = midiOutput.getAvailableDevices();
    int index = 1;
    
    for (const auto& device : devices)
    {
        midiDeviceSelector.addItem(device.name, index++);
    }
}

void StandaloneApp::midiDeviceChanged()
{
    if (midiDeviceSelector.getSelectedId() > 0)
    {
        auto devices = midiOutput.getAvailableDevices();
        auto selectedDevice = devices[midiDeviceSelector.getSelectedItemIndex()];
        midiOutput.setOutputDevice(selectedDevice.identifier);
    }
} 