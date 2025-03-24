#include "StandaloneApp.h"

StandaloneApp::StandaloneApp()
{
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
    // Map axes to CC 1-6 on channel 1 (JUCE uses 1-16 for channels)
    for (int axisIndex = 0; axisIndex < GamepadManager::MAX_AXES; ++axisIndex)
    {
        axisMappings[axisIndex] = {
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
        buttonMappings[buttonIndex] = {
            1, // channel (1-based for JUCE)
            buttonIndex + 20, // CC number
            0, // min value
            127, // max value
            true // is a button
        };
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
