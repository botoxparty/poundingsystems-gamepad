#pragma once

#include "PluginProcessor.h"
#include "GamepadComponent.h"
#include "BinaryData.h"
#include "melatonin_inspector/melatonin_inspector.h"

//==============================================================================
class PluginEditor : public juce::AudioProcessorEditor,
                     private juce::Timer
{
public:
    explicit PluginEditor (PluginProcessor&);
    ~PluginEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // Timer callback to check for new gamepads
    void timerCallback() override;

    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    PluginProcessor& processorRef;
    
    // Inspector for UI debugging
    std::unique_ptr<melatonin::Inspector> inspector;
    juce::TextButton inspectButton { "Inspect UI" };
    
    // Title of the plugin
    juce::Label titleLabel;
    
    // Instructions label
    juce::Label instructionsLabel;
    
    // Status label for connection info
    juce::Label statusLabel;
    
    // Components to display gamepads
    std::array<std::unique_ptr<GamepadComponent>, GamepadManager::MAX_GAMEPADS> gamepadComponents;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginEditor)
};
