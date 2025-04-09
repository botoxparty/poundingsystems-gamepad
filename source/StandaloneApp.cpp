#include "StandaloneApp.h"
#include "components/MidiMappingEditorWindow.h"

StandaloneApp::StandaloneApp()
{
    // Initialize MIDI output manager early
    MidiOutputManager::getInstance();
    
    // Set up MIDI mappings first
    setupMidiMappings();
    
    // Create single gamepad component
    gamepadComponent = std::make_unique<ModernGamepadComponent>(gamepadManager.getGamepadState(0), *this);
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
    midiMappingButton.setButtonText("Map");
    midiMappingButton.addListener(this);
    addAndMakeVisible(midiMappingButton);
    
    // Add components
    addAndMakeVisible(logoComponent);
    
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
    
    // Top area for MIDI device selector and mapping button
    auto topHeight = 40;
    auto topArea = area.removeFromTop(topHeight).reduced(5, 0);
    
    // Split the top area into two parts: selector and button
    auto buttonWidth = 100;
    auto selectorArea = topArea.removeFromLeft(topArea.getWidth() - buttonWidth - 10);
    auto buttonArea = topArea.reduced(0, 5); // Add vertical padding to the button
    
    midiDeviceSelector->setBounds(selectorArea);
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

    // If in MIDI learn mode, only process UI-triggered changes
    if (gamepadComponent->isMidiLearnMode())
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
            // Send MIDI messages for each mapping
            for (const auto& mapping : buttonMappings[static_cast<size_t>(i)])
            {
                if (mapping.type == MidiMapping::Type::ControlChange)
                {
                    int midiValue = currentState ? static_cast<int>(mapping.maxValue) : static_cast<int>(mapping.minValue);
                    MidiOutputManager::getInstance().sendControlChange(mapping.channel, mapping.ccNumber, midiValue);
                }
                else // Note
                {
                    // For buttons, we send note on when pressed and note off when released
                    if (currentState)
                    {
                        MidiOutputManager::getInstance().sendNoteOn(mapping.channel, mapping.noteNumber, mapping.maxValue / 127.0f);
                    }
                    else
                    {
                        // Send note off with zero velocity
                        MidiOutputManager::getInstance().sendNoteOn(mapping.channel, mapping.noteNumber, 0.0f);
                    }
                }
            }
            
            previousGamepadState.buttons[i] = currentState;
        }
    }

    // Process gyroscope changes
    if (gamepad.gyroscope.enabled)
    {
        float gyroValues[3] = {gamepad.gyroscope.x, gamepad.gyroscope.y, gamepad.gyroscope.z};
        float prevGyroValues[3] = {previousGamepadState.gyroscope.x, previousGamepadState.gyroscope.y, previousGamepadState.gyroscope.z};
        
        for (int i = 0; i < 3; ++i)
        {
            if (std::abs(gyroValues[i] - prevGyroValues[i]) > 0.01f)
            {
                // Send MIDI CC for each mapping
                for (const auto& mapping : gyroMappings[static_cast<size_t>(i)])
                {
                    float normalizedValue = (gyroValues[i] + 1.0f) * 0.5f; // Convert from [-1,1] to [0,1]
                    float mappedValue = mapping.minValue + (normalizedValue * (mapping.maxValue - mapping.minValue));
                    int midiValue = static_cast<int>(mappedValue);
                    
                    MidiOutputManager::getInstance().sendControlChange(mapping.channel, mapping.ccNumber, midiValue);
                }
                
                prevGyroValues[i] = gyroValues[i];
            }
        }
        
        previousGamepadState.gyroscope.x = gyroValues[0];
        previousGamepadState.gyroscope.y = gyroValues[1];
        previousGamepadState.gyroscope.z = gyroValues[2];
    }

    // Process accelerometer changes
    float accelValues[3] = {gamepad.accelerometer.x, gamepad.accelerometer.y, gamepad.accelerometer.z};
    float prevAccelValues[3] = {previousGamepadState.accelerometer.x, previousGamepadState.accelerometer.y, previousGamepadState.accelerometer.z};
    
    for (int i = 0; i < 3; ++i)
    {
        if (std::abs(accelValues[i] - prevAccelValues[i]) > 0.01f)
        {
            // Send MIDI CC for each mapping
            for (const auto& mapping : accelerometerMappings[static_cast<size_t>(i)])
            {
                float normalizedValue = (accelValues[i] + 1.0f) * 0.5f; // Convert from [-1,1] to [0,1]
                float mappedValue = mapping.minValue + (normalizedValue * (mapping.maxValue - mapping.minValue));
                int midiValue = static_cast<int>(mappedValue);
                
                MidiOutputManager::getInstance().sendControlChange(mapping.channel, mapping.ccNumber, midiValue);
            }
            
            prevAccelValues[i] = accelValues[i];
        }
    }
    
    previousGamepadState.accelerometer.x = accelValues[0];
    previousGamepadState.accelerometer.y = accelValues[1];
    previousGamepadState.accelerometer.z = accelValues[2];
}

void StandaloneApp::setupMidiMappings()
{
    // Initialize axis mappings with multiple MIDI CCs per control
    for (int i = 0; i < GamepadManager::MAX_AXES; ++i)
    {
        axisMappings[static_cast<size_t>(i)].clear();
        
        // Add default mapping for each axis
        MidiMapping defaultMapping;
        defaultMapping.type = MidiMapping::Type::ControlChange;
        defaultMapping.channel = 1;
        defaultMapping.ccNumber = i + 1; // CC 1-8 for axes
        defaultMapping.noteNumber = 0;  // Not used for CC
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
        defaultMapping.type = MidiMapping::Type::ControlChange;
        defaultMapping.channel = 1;
        defaultMapping.ccNumber = i + 16; // CC 16-31 for buttons
        defaultMapping.noteNumber = 0;  // Not used for CC
        defaultMapping.minValue = 0;
        defaultMapping.maxValue = 127;
        defaultMapping.isButton = true;
        
        buttonMappings[static_cast<size_t>(i)].push_back(defaultMapping);
    }

    // Initialize gyroscope mappings
    for (int i = 0; i < 3; ++i)
    {
        gyroMappings[static_cast<size_t>(i)].clear();
        
        MidiMapping defaultMapping;
        defaultMapping.type = MidiMapping::Type::ControlChange;
        defaultMapping.channel = 1;
        defaultMapping.ccNumber = MidiCC::GYRO_X + i; // CC 39-41 for gyro
        defaultMapping.noteNumber = 0;  // Not used for CC
        defaultMapping.minValue = 0;
        defaultMapping.maxValue = 127;
        defaultMapping.isButton = false;
        
        gyroMappings[static_cast<size_t>(i)].push_back(defaultMapping);
    }

    // Initialize accelerometer mappings
    for (int i = 0; i < 3; ++i)
    {
        accelerometerMappings[static_cast<size_t>(i)].clear();
        
        MidiMapping defaultMapping;
        defaultMapping.type = MidiMapping::Type::ControlChange;
        defaultMapping.channel = 1;
        defaultMapping.ccNumber = MidiCC::ACCEL_X + i; // CC 42-44 for accelerometer
        defaultMapping.noteNumber = 0;  // Not used for CC
        defaultMapping.minValue = 0;
        defaultMapping.maxValue = 127;
        defaultMapping.isButton = false;
        
        accelerometerMappings[static_cast<size_t>(i)].push_back(defaultMapping);
    }
}

void StandaloneApp::handleLogoClick()
{
    auto* aboutWindow = new AboutWindow();
    aboutWindow->enterModalState(true, nullptr, true);
}

void StandaloneApp::openMidiMappingEditor()
{
    // Create a new window that will delete itself when closed
    auto* window = new MidiMappingEditorWindow(*this);
    window->setVisible(true);
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

void StandaloneApp::notifyGamepadControlActivated(const juce::String& controlType, int controlIndex)
{
    // Find the MIDI editor window if it's open
    for (int i = 0; i < juce::TopLevelWindow::getNumTopLevelWindows(); ++i)
    {
        if (auto* window = dynamic_cast<MidiMappingEditorWindow*>(juce::TopLevelWindow::getTopLevelWindow(i)))
        {
            // Get the editor component from the window
            if (auto* editor = dynamic_cast<MidiMappingEditor*>(window->getContentComponent()))
            {
                // Highlight the control in the editor
                editor->highlightControl(controlType, controlIndex);
                break;
            }
        }
    }
}
