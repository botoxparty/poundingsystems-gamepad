#include "StandaloneApp.h"
#include "components/MidiMappingEditorWindow.h"

StandaloneApp::StandaloneApp()
{
    // Initialize MIDI output manager early
    MidiOutputManager::getInstance();
    
    // Create single gamepad component
    gamepadComponent = std::make_unique<ModernGamepadComponent>(gamepadManager.getGamepadState(0));
    addAndMakeVisible(gamepadComponent.get());  // Make visible immediately
    
    // Create MIDI device selector
    midiDeviceSelector = std::make_unique<MidiDeviceSelector>();
    addAndMakeVisible(midiDeviceSelector.get());
    
    // Set up logo
    auto logoImage = juce::ImageCache::getFromMemory(BinaryData::PoundingSystemsLogo_png, BinaryData::PoundingSystemsLogo_pngSize);
    logoComponent.setImage(logoImage);
    logoComponent.setImagePlacement(juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize);
    logoComponent.setMouseCursor(juce::MouseCursor::PointingHandCursor);
    logoComponent.addMouseListener(this, false);
    
    // Set up MIDI mapping button
    midiMappingButton.setButtonText("MIDI Mapping");
    midiMappingButton.addListener(this);
    addAndMakeVisible(midiMappingButton);
    
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
    
    // MIDI device selector at the top
    auto selectorHeight = 40;
    auto selectorArea = area.removeFromTop(selectorHeight).reduced(5, 0);
    midiDeviceSelector->setBounds(selectorArea);
    
    // MIDI mapping button below the selector
    auto buttonHeight = 30;
    auto buttonArea = area.removeFromTop(buttonHeight).reduced(5, 0);
    midiMappingButton.setBounds(buttonArea);
    
    // Gamepad area with padding
    auto gamepadArea = area.reduced(5, 0);
    gamepadComponent->setBounds(gamepadArea);
}

void StandaloneApp::timerCallback()
{
    // Always update the gamepad component
    gamepadComponent->updateState(gamepadManager.getGamepadState(0));
}

void StandaloneApp::handleGamepadStateChange()
{
    const auto& gamepad = gamepadManager.getGamepadState(0);
    if (!gamepad.connected)
        return;

    // Process axis changes
    for (int i = 0; i < GamepadManager::MAX_AXES; ++i)
    {
        float currentValue = gamepad.axes[i];
        float previousValue = previousGamepadState.axes[i];
        
        if (std::abs(currentValue - previousValue) > 0.01f)
        {
            // Send MIDI CC for each mapping
            for (const auto& mapping : axisMappings[static_cast<size_t>(i)])
            {
                float normalizedValue = (currentValue + 1.0f) * 0.5f; // Convert from [-1,1] to [0,1]
                float mappedValue = mapping.minValue + (normalizedValue * (mapping.maxValue - mapping.minValue));
                int midiValue = static_cast<int>(mappedValue);
                
                MidiOutputManager::getInstance().sendControlChange(mapping.channel, mapping.ccNumber, midiValue);
            }
            
            previousGamepadState.axes[i] = currentValue;
        }
    }

    // Process button changes
    for (int i = 0; i < GamepadManager::MAX_BUTTONS; ++i)
    {
        bool currentState = gamepad.buttons[i];
        bool previousState = previousGamepadState.buttons[i];
        
        if (currentState != previousState)
        {
            // Send MIDI CC for each mapping
            for (const auto& mapping : buttonMappings[static_cast<size_t>(i)])
            {
                int midiValue = currentState ? static_cast<int>(mapping.maxValue) : static_cast<int>(mapping.minValue);
                MidiOutputManager::getInstance().sendControlChange(mapping.channel, mapping.ccNumber, midiValue);
            }
            
            previousGamepadState.buttons[i] = currentState;
        }
    }

    // Process touchpad changes
    const auto& currentTouchpad = gamepad.touchpad;
    auto& previousTouchpad = previousGamepadState.touchpad;
    
    if (currentTouchpad.touched != previousTouchpad.touched ||
        currentTouchpad.pressed != previousTouchpad.pressed ||
        std::abs(currentTouchpad.x - previousTouchpad.x) > 0.01f ||
        std::abs(currentTouchpad.y - previousTouchpad.y) > 0.01f ||
        std::abs(currentTouchpad.pressure - previousTouchpad.pressure) > 0.01f)
    {
        // Send MIDI CC for touchpad X
        float normalizedX = (currentTouchpad.x + 1.0f) * 0.5f;
        int midiX = static_cast<int>(normalizedX * 127.0f);
        MidiOutputManager::getInstance().sendControlChange(1, MidiCC::TOUCHPAD_X, midiX);
        
        // Send MIDI CC for touchpad Y
        float normalizedY = (currentTouchpad.y + 1.0f) * 0.5f;
        int midiY = static_cast<int>(normalizedY * 127.0f);
        MidiOutputManager::getInstance().sendControlChange(1, MidiCC::TOUCHPAD_Y, midiY);
        
        // Send MIDI CC for touchpad pressure
        int midiPressure = static_cast<int>(currentTouchpad.pressure * 127.0f);
        MidiOutputManager::getInstance().sendControlChange(1, MidiCC::TOUCHPAD_PRESSURE, midiPressure);
        
        // Copy touchpad state fields individually
        previousTouchpad.touched = currentTouchpad.touched;
        previousTouchpad.pressed = currentTouchpad.pressed;
        previousTouchpad.x = currentTouchpad.x;
        previousTouchpad.y = currentTouchpad.y;
        previousTouchpad.pressure = currentTouchpad.pressure;
    }

    // Process gyroscope changes
    const auto& currentGyro = gamepad.gyroscope;
    auto& previousGyro = previousGamepadState.gyroscope;
    
    if (currentGyro.enabled != previousGyro.enabled ||
        std::abs(currentGyro.x - previousGyro.x) > 0.01f ||
        std::abs(currentGyro.y - previousGyro.y) > 0.01f ||
        std::abs(currentGyro.z - previousGyro.z) > 0.01f)
    {
        // Send MIDI CC for gyroscope X
        float normalizedX = (currentGyro.x + 1.0f) * 0.5f;
        int midiX = static_cast<int>(normalizedX * 127.0f);
        MidiOutputManager::getInstance().sendControlChange(1, MidiCC::GYRO_X, midiX);
        
        // Send MIDI CC for gyroscope Y
        float normalizedY = (currentGyro.y + 1.0f) * 0.5f;
        int midiY = static_cast<int>(normalizedY * 127.0f);
        MidiOutputManager::getInstance().sendControlChange(1, MidiCC::GYRO_Y, midiY);
        
        // Send MIDI CC for gyroscope Z
        float normalizedZ = (currentGyro.z + 1.0f) * 0.5f;
        int midiZ = static_cast<int>(normalizedZ * 127.0f);
        MidiOutputManager::getInstance().sendControlChange(1, MidiCC::GYRO_Z, midiZ);
        
        // Copy gyroscope state fields individually
        previousGyro.enabled = currentGyro.enabled;
        previousGyro.x = currentGyro.x;
        previousGyro.y = currentGyro.y;
        previousGyro.z = currentGyro.z;
    }
}

void StandaloneApp::setupMidiMappings()
{
    // Initialize axis mappings with multiple MIDI CCs per control
    for (int i = 0; i < GamepadManager::MAX_AXES; ++i)
    {
        axisMappings[static_cast<size_t>(i)].clear();
        
        // Add default mapping for each axis
        MidiMapping defaultMapping;
        defaultMapping.channel = 1;
        defaultMapping.ccNumber = i + 1; // CC 1-8 for axes
        defaultMapping.minValue = 0;
        defaultMapping.maxValue = 127;
        defaultMapping.isButton = false;
        
        axisMappings[static_cast<size_t>(i)].push_back(defaultMapping);
    }
    
    // Initialize button mappings with multiple MIDI CCs per control
    for (int i = 0; i < GamepadManager::MAX_BUTTONS; ++i)
    {
        buttonMappings[static_cast<size_t>(i)].clear();
        
        // Add default mapping for each button
        MidiMapping defaultMapping;
        defaultMapping.channel = 1;
        defaultMapping.ccNumber = i + 16; // CC 16-31 for buttons
        defaultMapping.minValue = 0;
        defaultMapping.maxValue = 127;
        defaultMapping.isButton = true;
        
        buttonMappings[static_cast<size_t>(i)].push_back(defaultMapping);
    }
}

void StandaloneApp::handleLogoClick()
{
    auto* aboutWindow = new AboutWindow();
    aboutWindow->enterModalState(true, nullptr, true);
}

void StandaloneApp::openMidiMappingEditor()
{
    if (midiMappingWindow == nullptr)
    {
        midiMappingWindow = std::make_unique<MidiMappingEditorWindow>(*this);
    }
    
    midiMappingWindow->enterModalState(true, nullptr, true);
}

void StandaloneApp::buttonClicked(juce::Button* button)
{
    if (button == &midiMappingButton)
    {
        openMidiMappingEditor();
    }
}

void StandaloneApp::mouseUp(const juce::MouseEvent& event)
{
    if (event.eventComponent == &logoComponent)
    {
        handleLogoClick();
    }
}
