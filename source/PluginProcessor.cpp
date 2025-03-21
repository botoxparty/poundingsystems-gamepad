#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
PluginProcessor::PluginProcessor()
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
{
    // Create and set up the parameter tree
    setParameterTree(createParameters());
    
    // Register a callback to update parameters when gamepad state changes
    gamepadManager.addStateChangeCallback([this]() { updateParametersFromGamepadState(); });
}

PluginProcessor::~PluginProcessor()
{
}

juce::AudioProcessorParameterGroup PluginProcessor::createParameters()
{
    // Create a root group for all parameters
    auto rootGroup = juce::AudioProcessorParameterGroup("root", "Root", "|");
    
    // Create parameters for each potential gamepad
    for (GamepadIndex gamepadIndex = 0; gamepadIndex < static_cast<GamepadIndex>(GamepadManager::MAX_GAMEPADS); ++gamepadIndex)
    {
        juce::String gamepadPrefix = "Gamepad" + juce::String(gamepadIndex + 1) + "_";
        auto gamepadGroup = juce::AudioProcessorParameterGroup(
            gamepadPrefix, "Gamepad " + juce::String(gamepadIndex + 1), "/");
        
        // Create axis parameters
        for (AxisIndex axisIndex = 0; axisIndex < static_cast<AxisIndex>(GamepadManager::MAX_AXES); ++axisIndex)
        {
            juce::String axisName;
            
            // Assign human-readable names to axes
            switch (axisIndex)
            {
                case 0: axisName = "Left Stick X"; break;
                case 1: axisName = "Left Stick Y"; break;
                case 2: axisName = "Right Stick X"; break;
                case 3: axisName = "Right Stick Y"; break;
                case 4: axisName = "Left Trigger"; break;
                case 5: axisName = "Right Trigger"; break;
                default: axisName = "Axis " + juce::String(static_cast<int>(axisIndex)); break;
            }
            
            juce::String paramId = gamepadPrefix + "Axis" + juce::String(static_cast<int>(axisIndex));
            juce::String paramName = gamepadPrefix + axisName;
            
            auto* param = new juce::AudioParameterFloat(
                paramId, paramName, -1.0f, 1.0f, 0.0f);
                
            axisParameters[gamepadIndex][axisIndex] = param;
            gamepadGroup.addChild(std::unique_ptr<juce::AudioParameterFloat>(param));
        }
        
        // Create button parameters
        for (ButtonIndex buttonIndex = 0; buttonIndex < static_cast<ButtonIndex>(GamepadManager::MAX_BUTTONS); ++buttonIndex)
        {
            juce::String buttonName;
            
            // Assign human-readable names to buttons
            switch (buttonIndex)
            {
                case 0: buttonName = "A"; break;
                case 1: buttonName = "B"; break;
                case 2: buttonName = "X"; break;
                case 3: buttonName = "Y"; break;
                case 4: buttonName = "Back"; break;
                case 5: buttonName = "Guide"; break;
                case 6: buttonName = "Start"; break;
                case 7: buttonName = "Left Stick"; break;
                case 8: buttonName = "Right Stick"; break;
                case 9: buttonName = "Left Shoulder"; break;
                case 10: buttonName = "Right Shoulder"; break;
                case 11: buttonName = "DPad Up"; break;
                case 12: buttonName = "DPad Down"; break;
                case 13: buttonName = "DPad Left"; break;
                case 14: buttonName = "DPad Right"; break;
                default: buttonName = "Button " + juce::String(static_cast<int>(buttonIndex)); break;
            }
            
            juce::String paramId = gamepadPrefix + "Button" + juce::String(static_cast<int>(buttonIndex));
            juce::String paramName = gamepadPrefix + buttonName;
            
            auto* param = new juce::AudioParameterBool(
                paramId, paramName, false);
                
            buttonParameters[gamepadIndex][buttonIndex] = param;
            gamepadGroup.addChild(std::unique_ptr<juce::AudioParameterBool>(param));
        }
        
        rootGroup.addChild(std::make_unique<juce::AudioProcessorParameterGroup>(std::move(gamepadGroup)));
    }
    
    return rootGroup;
}

void PluginProcessor::updateParametersFromGamepadState()
{
    for (GamepadIndex gamepadIndex = 0; gamepadIndex < static_cast<GamepadIndex>(GamepadManager::MAX_GAMEPADS); ++gamepadIndex)
    {
        const auto& gamepadState = gamepadManager.getGamepadState(static_cast<int>(gamepadIndex));
        
        // Update axis parameters
        for (AxisIndex axisIndex = 0; axisIndex < static_cast<AxisIndex>(GamepadManager::MAX_AXES); ++axisIndex)
        {
            if (auto* param = axisParameters[gamepadIndex][axisIndex])
            {
                if (gamepadState.connected)
                {
                    *param = gamepadState.axes[axisIndex];
                }
                else
                {
                    *param = 0.0f;
                }
            }
        }
        
        // Update button parameters
        for (ButtonIndex buttonIndex = 0; buttonIndex < static_cast<ButtonIndex>(GamepadManager::MAX_BUTTONS); ++buttonIndex)
        {
            if (auto* param = buttonParameters[gamepadIndex][buttonIndex])
            {
                if (gamepadState.connected)
                {
                    *param = gamepadState.buttons[buttonIndex];
                }
                else
                {
                    *param = false;
                }
            }
        }
    }
}

juce::AudioParameterFloat* PluginProcessor::getAxisParameter(int gamepadIndex, int axisIndex)
{
    if (gamepadIndex >= 0 && static_cast<GamepadIndex>(gamepadIndex) < GamepadManager::MAX_GAMEPADS &&
        axisIndex >= 0 && static_cast<AxisIndex>(axisIndex) < GamepadManager::MAX_AXES)
    {
        return axisParameters[static_cast<GamepadIndex>(gamepadIndex)][static_cast<AxisIndex>(axisIndex)];
    }
    return nullptr;
}

juce::AudioParameterBool* PluginProcessor::getButtonParameter(int gamepadIndex, int buttonIndex)
{
    if (gamepadIndex >= 0 && static_cast<GamepadIndex>(gamepadIndex) < GamepadManager::MAX_GAMEPADS &&
        buttonIndex >= 0 && static_cast<ButtonIndex>(buttonIndex) < GamepadManager::MAX_BUTTONS)
    {
        return buttonParameters[static_cast<GamepadIndex>(gamepadIndex)][static_cast<ButtonIndex>(buttonIndex)];
    }
    return nullptr;
}

//==============================================================================
const juce::String PluginProcessor::getName() const
{
    return JucePlugin_Name;
}

bool PluginProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool PluginProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool PluginProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double PluginProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int PluginProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int PluginProcessor::getCurrentProgram()
{
    return 0;
}

void PluginProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused (index);
}

const juce::String PluginProcessor::getProgramName (int index)
{
    juce::ignoreUnused (index);
    return {};
}

void PluginProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused (index, newName);
}

//==============================================================================
void PluginProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Store audio settings
    setRateAndBufferSizeDetails(sampleRate, samplesPerBlock);
}

void PluginProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

bool PluginProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}

void PluginProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                              juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused (midiMessages);

    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    // We're not processing audio in this plugin, it's just for parameter automation
    // The GamepadManager has its own timer to update states, so we don't need to call it here
    // This avoids potential thread safety issues
}

void PluginProcessor::processBlock (juce::AudioBuffer<double>& buffer,
                                               juce::MidiBuffer& midiMessages)
{
    // Double precision version - just pass through audio
    juce::ignoreUnused (midiMessages);
    
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, clear any output
    // channels that didn't contain input data
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
}

//==============================================================================
bool PluginProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* PluginProcessor::createEditor()
{
    return new PluginEditor (*this);
}

//==============================================================================
void PluginProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // We don't need to save any state as parameters are driven by the gamepad in real-time
    juce::ignoreUnused (destData);
}

void PluginProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // We don't need to restore any state as parameters are driven by the gamepad in real-time
    juce::ignoreUnused (data, sizeInBytes);
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PluginProcessor();
}
