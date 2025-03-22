#include "StandaloneApp.h"

StandaloneApp::StandaloneApp()
    : statusBar(midiOutput)
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
    addAndMakeVisible(statusBar);
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
    auto bounds = getLocalBounds().toFloat();
    
    // Create a sophisticated dark background
    g.fillAll(juce::Colour(22, 22, 26));
    
    // Create a multi-layer gradient effect for depth
    
    // Layer 1: Base radial gradient from center
    juce::ColourGradient baseGradient(
        juce::Colour(28, 28, 32).withAlpha(0.7f),
        bounds.getCentreX(), bounds.getCentreY(),
        juce::Colour(22, 22, 26),
        bounds.getRight(), bounds.getBottom(),
        true);  // Make it radial
    g.setGradientFill(baseGradient);
    g.fillAll();
    
    // Layer 2: Subtle top-to-bottom gradient overlay
    juce::ColourGradient overlayGradient(
        juce::Colour(30, 30, 34).withAlpha(0.2f),
        0.0f, 0.0f,
        juce::Colour(25, 25, 29).withAlpha(0.2f),
        0.0f, bounds.getHeight(),
        false);
    g.setGradientFill(overlayGradient);
    g.fillAll();
    
    // Layer 3: Very subtle edge lighting effect
    juce::ColourGradient edgeGlow(
        juce::Colours::white.withAlpha(0.02f),
        bounds.getX(), bounds.getY(),
        juce::Colours::transparentWhite,
        bounds.getX() + 50.0f, bounds.getY() + 50.0f,
        true);
    g.setGradientFill(edgeGlow);
    g.fillRect(bounds.removeFromTop(100.0f));
}

void StandaloneApp::resized()
{
    auto area = getLocalBounds();
    
    // Status bar at the top
    auto statusBarHeight = 45;  // Slightly taller for more presence
    auto statusBarArea = area.removeFromTop(statusBarHeight);
    statusBar.setBounds(statusBarArea);
    
    // Add padding after status bar
    area.removeFromTop(15);
    
    // Footer with logo
    auto logoHeight = 35;  // Taller logo area
    auto footerArea = area.removeFromBottom(logoHeight + 25);  // More padding for footer
    
    // Create a sleek container for the logo
    auto logoArea = footerArea.reduced(20, 5);
    logoComponent.setBounds(logoArea);
    
    // Gamepad area with modern spacing
    auto gamepadArea = area.reduced(25, 0);  // More horizontal padding for better framing
    int connectedGamepads = gamepadManager.getNumConnectedGamepads();
    
    if (connectedGamepads > 0)
    {
        int heightPerGamepad = gamepadArea.getHeight() / connectedGamepads;
        
        for (int i = 0; i < GamepadManager::MAX_GAMEPADS; ++i)
        {
            if (gamepadManager.isGamepadConnected(i))
            {
                // Modern spacing between gamepads
                auto componentBounds = gamepadArea.removeFromTop(heightPerGamepad).reduced(0, 8);
                gamepadComponents[i]->setBounds(componentBounds);
                gamepadComponents[i]->setVisible(true);
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
    
    // Update status bar
    statusBar.setNumGamepads(gamepadManager.getNumConnectedGamepads());
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
