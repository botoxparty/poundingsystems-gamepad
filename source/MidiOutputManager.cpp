#include "MidiOutputManager.h"

MidiOutputManager::MidiOutputManager()
{
    // Always create virtual device first
    if (!createVirtualDevice())
    {
        juce::String errorMessage = "Failed to create virtual MIDI device.\n\n"
                                  "This is likely a permissions or entitlements issue.\n"
                                  "Make sure your app is not sandboxed and has proper entitlements.";
        
        juce::Logger::writeToLog(errorMessage);
        
        // Show error popup
        juce::NativeMessageBox::showMessageBoxAsync(
            juce::MessageBoxIconType::WarningIcon,
            "MIDI Device Error",
            errorMessage);
    }
    
    // Set virtual device as default
    setOutputDevice(virtualDeviceInfo.identifier);
}

bool MidiOutputManager::createVirtualDevice()
{
    // Create virtual MIDI device
    virtualDevice = juce::MidiOutput::createNewDevice("Gamepad MIDI");
    
    if (virtualDevice != nullptr)
    {
        virtualDeviceInfo = virtualDevice->getDeviceInfo();
        virtualDevice->startBackgroundThread();
        
        juce::Logger::writeToLog("Virtual MIDI device created successfully");
        juce::Logger::writeToLog("Device name: " + virtualDeviceInfo.name);
        juce::Logger::writeToLog("Device identifier: " + virtualDeviceInfo.identifier);
        return true;
    }
    
    juce::Logger::writeToLog("ERROR: Failed to create virtual MIDI device!");
    return false;
}

bool MidiOutputManager::isVirtualDevice(const juce::String& identifier) const
{
    return identifier == virtualDeviceInfo.identifier;
}

MidiOutputManager::~MidiOutputManager()
{
    closeCurrentDevice();
    if (virtualDevice != nullptr)
    {
        virtualDevice->stopBackgroundThread();
        virtualDevice.reset();
    }
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
    
    juce::Logger::writeToLog("Attempting to open device: " + identifier);
    juce::Logger::writeToLog("Virtual device info - Name: " + virtualDeviceInfo.name + 
                            ", ID: " + virtualDeviceInfo.identifier);
    
    // If this is the virtual device, use our stored reference
    if (isVirtualDevice(identifier))
    {
        juce::Logger::writeToLog("Opening virtual device");
        // Instead of moving the virtual device, create a new instance for output
        midiOutput = juce::MidiOutput::openDevice(identifier);
        if (midiOutput != nullptr)
        {
            currentDeviceInfo = virtualDeviceInfo;
            midiOutput->startBackgroundThread();
            return true;
        }
        return false;
    }
    
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
    {
        juce::Logger::writeToLog("Device already selected: " + identifier);
        return true;
    }
    
    juce::Logger::writeToLog("Setting output device to: " + identifier);
    bool success = openDevice(identifier);
    if (success)
    {
        juce::Logger::writeToLog("Successfully set output device to: " + currentDeviceInfo.name);
    }
    else
    {
        juce::Logger::writeToLog("Failed to set output device to: " + identifier);
    }
    return success;
}

juce::Array<juce::MidiDeviceInfo> MidiOutputManager::getAvailableDevices() const
{
    auto devices = juce::MidiOutput::getAvailableDevices();
    
    juce::Logger::writeToLog("Physical MIDI devices:");
    for (const auto& device : devices)
    {
        juce::Logger::writeToLog(" - " + device.name + " (" + device.identifier + ")");
    }
    
    // Add virtual device to the list if it exists
    if (virtualDevice != nullptr)
    {
        juce::Logger::writeToLog("Adding virtual device to list: " + virtualDeviceInfo.name + " (" + virtualDeviceInfo.identifier + ")");
        devices.add(virtualDeviceInfo);
    }
    else
    {
        juce::Logger::writeToLog("WARNING: Virtual device is null!");
    }
    
    return devices;
}

void MidiOutputManager::sendControlChange(int channel, int controller, int value)
{
    juce::Logger::writeToLog("Attempting to send MIDI CC - Device: " + (midiOutput != nullptr ? currentDeviceInfo.name : "None") + 
                            ", Channel: " + juce::String(channel) + 
                            ", Controller: " + juce::String(controller) + 
                            ", Value: " + juce::String(value));
    
    if (midiOutput != nullptr)
    {
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
        juce::Logger::writeToLog("MIDI CC message sent successfully");
    }
    else
    {
        juce::Logger::writeToLog("WARNING: No MIDI output device selected! Current device info: " + 
                                currentDeviceInfo.name + " (" + currentDeviceInfo.identifier + ")");
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