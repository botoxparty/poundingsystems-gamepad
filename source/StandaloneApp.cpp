#include "StandaloneApp.h"

StandaloneApp::StandaloneApp()
{
    // Create gamepad components
    for (int i = 0; i < GamepadManager::MAX_GAMEPADS; ++i)
    {
        gamepadComponents[i] = std::make_unique<GamepadComponent>(
            gamepadManager.getGamepadState(i));
        addChildComponent(gamepadComponents[i].get());
    }
    
    // Set up logo
    auto logoImage = juce::ImageCache::getFromMemory(BinaryData::PoundingSystemsLogo_png, BinaryData::PoundingSystemsLogo_pngSize);
    logoComponent.setImage(logoImage);
    logoComponent.setImagePlacement(juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize);
    
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
    // Light gray background
    g.fillAll(juce::Colour(230, 230, 230));  // Slightly darker gray for better contrast
    
    // Simple border
    g.setColour(juce::Colours::darkgrey);
    g.drawRect(getLocalBounds().toFloat(), 1.0f);
}

void StandaloneApp::resized()
{
    auto area = getLocalBounds();
    
    // Footer with logo
    auto logoHeight = 35;
    auto footerArea = area.removeFromBottom(logoHeight + 25);
    auto logoArea = footerArea.reduced(20, 5);
    logoComponent.setBounds(logoArea);
    
    // Gamepad area with padding
    auto gamepadArea = area.reduced(25, 15);  // Add vertical padding
    int connectedGamepads = gamepadManager.getNumConnectedGamepads();
    
    if (connectedGamepads > 0)
    {
        int heightPerGamepad = (gamepadArea.getHeight() - (connectedGamepads - 1) * 10) / connectedGamepads;
        int visibleCount = 0;
        
        for (int i = 0; i < GamepadManager::MAX_GAMEPADS; ++i)
        {
            if (gamepadManager.isGamepadConnected(i))
            {
                // Add spacing between gamepads
                if (visibleCount > 0)
                    gamepadArea.removeFromTop(10);
                
                auto componentBounds = gamepadArea.removeFromTop(heightPerGamepad);
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
                    const auto& mapping = buttonMappings[gamepadIndex][buttonIndex];
                    int midiValue = currentState.buttons[buttonIndex] ? mapping.maxValue : mapping.minValue;
                    
                    MidiOutputManager::getInstance().sendControlChange(mapping.channel, mapping.ccNumber, midiValue);
                    
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
