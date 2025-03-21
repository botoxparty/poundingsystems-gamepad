#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "GamepadManager.h"

#if (MSVC)
#include "ipps.h"
#endif

class PluginProcessor : public juce::AudioProcessor
{
public:
    PluginProcessor();
    ~PluginProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    void processBlock (juce::AudioBuffer<double>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;  // This prevents hiding the base class versions

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    // Returns the current gamepad manager
    GamepadManager& getGamepadManager() { return gamepadManager; }
    
    // Returns the number of active gamepad devices
    int getNumGamepads() const { return gamepadManager.getNumConnectedGamepads(); }
    
    // Returns the AudioParameterFloat for a specific gamepad axis
    juce::AudioParameterFloat* getAxisParameter(int gamepadIndex, int axisIndex);
    
    // Returns the AudioParameterBool for a specific gamepad button
    juce::AudioParameterBool* getButtonParameter(int gamepadIndex, int buttonIndex);

private:
    // Create the parameter tree
    juce::AudioProcessorParameterGroup createParameters();
    
    // Updates all parameter values based on current gamepad state
    void updateParametersFromGamepadState();
    
    // The GamepadManager handles SDL initialization and gamepad input
    GamepadManager gamepadManager;
    
    // Type definitions for array indices
    using AxisIndex = std::size_t;
    using ButtonIndex = std::size_t;
    using GamepadIndex = std::size_t;
    
    // Arrays to store parameter pointers (owned by the AudioProcessor parameter tree)
    using GamepadAxisParams = std::array<juce::AudioParameterFloat*, GamepadManager::MAX_AXES>;
    using GamepadButtonParams = std::array<juce::AudioParameterBool*, GamepadManager::MAX_BUTTONS>;
    
    std::array<GamepadAxisParams, GamepadManager::MAX_GAMEPADS> axisParameters;
    std::array<GamepadButtonParams, GamepadManager::MAX_GAMEPADS> buttonParameters;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginProcessor)
};
