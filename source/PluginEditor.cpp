#include "PluginEditor.h"

PluginEditor::PluginEditor (PluginProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    // Set up title label
    titleLabel.setText("Gamepad DAW Interface", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(24.0f, juce::Font::bold));
    titleLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(titleLabel);
    
    // Set up instructions label
    instructionsLabel.setText(
        "This plugin allows you to control your DAW with a gamepad.\n"
        "Connect a gamepad and use the parameters in your DAW's automation controls.\n"
        "Each axis and button is exposed as an automatable parameter.", 
        juce::dontSendNotification);
    instructionsLabel.setFont(juce::Font(14.0f));
    instructionsLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(instructionsLabel);
    
    // Set up status label
    statusLabel.setText("No gamepads connected", juce::dontSendNotification);
    statusLabel.setFont(juce::Font(16.0f, juce::Font::bold));
    statusLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(statusLabel);
    
    // Add inspect button
    addAndMakeVisible(inspectButton);
    inspectButton.onClick = [&] {
        if (!inspector)
        {
            inspector = std::make_unique<melatonin::Inspector>(*this);
            inspector->onClose = [this]() { inspector.reset(); };
        }
        inspector->setVisible(true);
    };
    
    // Create gamepad components
    for (int i = 0; i < GamepadManager::MAX_GAMEPADS; ++i)
    {
        gamepadComponents[i] = std::make_unique<GamepadComponent>(
            processorRef.getGamepadManager().getGamepadState(i));
        addChildComponent(gamepadComponents[i].get());
    }
    
    // Start timer to check for gamepad connections (every 500ms)
    startTimer(500);

    // Set the plugin's window size
    setSize(800, 600);
}

PluginEditor::~PluginEditor()
{
    stopTimer();
}

void PluginEditor::paint (juce::Graphics& g)
{
    // Fill the background
    g.fillAll(juce::Colour(30, 30, 30));
}

void PluginEditor::resized()
{
    auto area = getLocalBounds().reduced(20);
    
    // Position title at the top
    titleLabel.setBounds(area.removeFromTop(40));
    
    // Position instructions below the title
    instructionsLabel.setBounds(area.removeFromTop(60));
    
    // Position status label
    statusLabel.setBounds(area.removeFromTop(30));
    
    // Calculate area for gamepad displays
    auto gamepadArea = area.removeFromBottom(area.getHeight() - 40);
    
    // Position inspect button
    inspectButton.setBounds(area.removeFromBottom(30).withSizeKeepingCentre(120, 30));
    
    // Count connected gamepads
    int connectedGamepads = processorRef.getNumGamepads();
    
    if (connectedGamepads > 0)
    {
        // Calculate height for each gamepad component
        int heightPerGamepad = gamepadArea.getHeight() / connectedGamepads;
        
        // Position each gamepad component
        int visibleCount = 0;
        for (int i = 0; i < GamepadManager::MAX_GAMEPADS; ++i)
        {
            if (processorRef.getGamepadManager().isGamepadConnected(i))
            {
                auto componentBounds = gamepadArea.removeFromTop(heightPerGamepad).reduced(10);
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

void PluginEditor::timerCallback()
{
    // Update status label based on connected gamepads
    int connectedGamepads = processorRef.getNumGamepads();
    
    if (connectedGamepads == 0)
    {
        statusLabel.setText("No gamepads connected", juce::dontSendNotification);
    }
    else
    {
        statusLabel.setText(juce::String(connectedGamepads) + " gamepad" + 
                          (connectedGamepads > 1 ? "s" : "") + " connected", 
                          juce::dontSendNotification);
    }
    
    // Make sure gamepad components are properly laid out
    resized();
}
