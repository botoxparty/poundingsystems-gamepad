#include "MidiOutputManager.h"

MidiOutputManager::MidiOutputManager()
{
    // Log available devices before creating virtual device
    auto devices = juce::MidiOutput::getAvailableDevices();
    juce::Logger::writeToLog("Available MIDI devices before creating virtual device:");
    for (const auto& device : devices)
        juce::Logger::writeToLog(" - " + device.name + " (" + device.identifier + ")");
    
    // Try to use first available device, if none available create virtual device
    if (devices.isEmpty())
        createInitialVirtualDevice();
    else
        setOutputDevice(devices[0].identifier);
}

void MidiOutputManager::createInitialVirtualDevice()
{
    // Create virtual MIDI device
    midiOutput = juce::MidiOutput::createNewDevice("Gamepad MIDI");
    
    if (midiOutput != nullptr)
    {
        currentDeviceInfo = midiOutput->getDeviceInfo();
        midiOutput->startBackgroundThread();
        
        juce::Logger::writeToLog("Virtual MIDI device created successfully");
        juce::Logger::writeToLog("Device name: " + currentDeviceInfo.name);
        juce::Logger::writeToLog("Device identifier: " + currentDeviceInfo.identifier);
    }
    else
    {
        juce::Logger::writeToLog("Failed to create virtual MIDI device");
        juce::Logger::writeToLog("This is likely a permissions or entitlements issue.");
        juce::Logger::writeToLog("Make sure your app is not sandboxed and has proper entitlements.");
    }
}

MidiOutputManager::~MidiOutputManager()
{
    closeCurrentDevice();
}

void MidiOutputManager::closeCurrentDevice()
{
    if (midiOutput != nullptr)
    {
        juce::Logger::writeToLog("Closing MIDI device: " + currentDeviceInfo.name);
        midiOutput->stopBackgroundThread();
        midiOutput.reset();
        currentDeviceInfo = juce::MidiDeviceInfo();
    }
}

bool MidiOutputManager::openDevice(const juce::String& identifier)
{
    // First close any existing device
    closeCurrentDevice();
    
    // Try to open the new device
    midiOutput = juce::MidiOutput::openDevice(identifier);
    
    if (midiOutput != nullptr)
    {
        currentDeviceInfo = midiOutput->getDeviceInfo();
        midiOutput->startBackgroundThread();
        
        juce::Logger::writeToLog("Opened MIDI device: " + currentDeviceInfo.name);
        return true;
    }
    
    juce::Logger::writeToLog("Failed to open MIDI device with identifier: " + identifier);
    return false;
}

bool MidiOutputManager::setOutputDevice(const juce::String& identifier)
{
    // If we're already using this device, do nothing
    if (currentDeviceInfo.identifier == identifier)
        return true;
        
    return openDevice(identifier);
}

juce::Array<juce::MidiDeviceInfo> MidiOutputManager::getAvailableDevices() const
{
    return juce::MidiOutput::getAvailableDevices();
}

void MidiOutputManager::sendControlChange(int channel, int controller, int value)
{
    if (midiOutput != nullptr)
    {
        // Add debug logging
        juce::Logger::writeToLog("MIDI CC - Channel: " + juce::String(channel) + 
                                  ", Controller: " + juce::String(controller) + 
                                  ", Value: " + juce::String(value));
        
        // Check if channel is valid (1-16)
        if (channel < 1 || channel > 16)
        {
            juce::Logger::writeToLog("WARNING: Invalid MIDI channel: " + juce::String(channel) + 
                                     " (must be 1-16). Correcting to channel 1.");
            channel = 1; // Default to channel 1 if invalid
        }
        
        // Check if controller number is valid (0-127)
        if (controller < 0 || controller > 127)
        {
            juce::Logger::writeToLog("WARNING: Invalid controller number: " + juce::String(controller) + 
                                     " (must be 0-127). Correcting to 0.");
            controller = 0; // Default to 0 if invalid
        }
        
        juce::MidiMessage message = juce::MidiMessage::controllerEvent(channel, controller, value);
        midiOutput->sendMessageNow(message);
    }
}

void MidiOutputManager::sendNoteOn(int channel, int noteNumber, float velocity)
{
    if (midiOutput != nullptr)
    {
        auto message = juce::MidiMessage::noteOn(channel, noteNumber, velocity);
        midiOutput->sendMessageNow(message);
    }
} 