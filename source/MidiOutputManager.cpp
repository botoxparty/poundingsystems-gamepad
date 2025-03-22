#include "MidiOutputManager.h"

MidiOutputManager::MidiOutputManager()
    : deviceListConnection(juce::MidiDeviceListConnection::make([this] { handleDevicesChanged(); }))
{
    // Log available devices before creating virtual device
    auto devices = getAvailableDevices();
    juce::Logger::writeToLog("Available MIDI devices before creating virtual device:");
    for (const auto& device : devices)
        juce::Logger::writeToLog(" - " + device.name + " (" + device.identifier + ")");
    
    // Create a virtual MIDI device
    if (!createVirtualDevice("Gamepad MIDI Controller"))
    {
        juce::Logger::writeToLog("Failed to create virtual MIDI device");
        
        // If we couldn't create a virtual device, let's check for the IAC Driver
        bool foundIAC = false;
        for (const auto& device : devices)
        {
            if (device.name.contains("IAC"))
            {
                juce::Logger::writeToLog("Found IAC Driver: " + device.name);
                juce::Logger::writeToLog("Consider using this instead of creating a virtual device");
                foundIAC = true;
                break;
            }
        }
        
        if (!foundIAC)
        {
            juce::Logger::writeToLog("No IAC Driver found.");
            juce::Logger::writeToLog("On macOS, you may need to enable the IAC Driver in Audio MIDI Setup.");
        }
    }
    else
    {
        juce::Logger::writeToLog("Successfully created virtual MIDI device");
        
        if (waitForDeviceToAppear())
        {
            juce::Logger::writeToLog("Virtual device successfully registered in system");
        }
        else
        {
            juce::Logger::writeToLog("Virtual device failed to appear in system");
            juce::Logger::writeToLog("This is likely a permissions or entitlements issue.");
        }
    }
    
    isInitializing = false;
}

bool MidiOutputManager::waitForDeviceToAppear()
{
    // Try for up to 3 seconds (6 attempts, 500ms each)
    for (int attempt = 0; attempt < 6; ++attempt)
    {
        juce::Thread::sleep(500);
        
        auto devices = getAvailableDevices();
        juce::Logger::writeToLog("Checking for virtual device (attempt " + juce::String(attempt + 1) + "):");
        
        for (const auto& device : devices)
        {
            juce::Logger::writeToLog(" - " + device.name + " (" + device.identifier + ")");
            
            // Check by identifier (most reliable)
            if (device.identifier == currentDeviceInfo.identifier)
            {
                juce::Logger::writeToLog("Found device by identifier match");
                return true;
            }
            
            // Alternative: check by name
            if (device.name == currentDeviceInfo.name)
            {
                juce::Logger::writeToLog("Found device by name match");
                // Update the identifier since it might have changed
                currentDeviceInfo = device;
                return true;
            }
        }
    }
    
    juce::Logger::writeToLog("Virtual device not found after multiple attempts");
    juce::Logger::writeToLog("Current device info: " + currentDeviceInfo.name + " (" + currentDeviceInfo.identifier + ")");
    return false;
}

MidiOutputManager::~MidiOutputManager()
{
    if (midiOutput != nullptr)
    {
        if (isVirtualDevice)
        {
            juce::Logger::writeToLog("Cleaning up virtual MIDI device");
            midiOutput->stopBackgroundThread();
        }
        midiOutput.reset();
    }
}

void MidiOutputManager::handleDevicesChanged()
{
    // Ignore device changes during initialization
    if (isInitializing)
        return;
        
    auto devices = getAvailableDevices();
    juce::Logger::writeToLog("MIDI devices changed, current devices:");
    for (const auto& device : devices)
        juce::Logger::writeToLog(" - " + device.name + " (" + device.identifier + ")");
        
    // If we have a virtual device, check if it's still in the list
    if (isVirtualDevice && midiOutput != nullptr)
    {
        bool found = false;
        for (const auto& device : devices)
        {
            if (device.identifier == currentDeviceInfo.identifier)
            {
                found = true;
                break;
            }
        }
        
        if (!found)
        {
            juce::Logger::writeToLog("Virtual device was removed, attempting to recreate");
            createVirtualDevice("Gamepad MIDI Controller");
        }
    }
}

juce::Array<juce::MidiDeviceInfo> MidiOutputManager::getAvailableDevices() const
{
    auto devices = juce::MidiOutput::getAvailableDevices();
    
    // Append the virtual device if it exists and is active
    if (isVirtualDevice && midiOutput != nullptr)
    {
        // Only add if it's not already in the list
        bool alreadyExists = false;
        for (const auto& device : devices)
        {
            if (device.identifier == currentDeviceInfo.identifier || 
                device.name == currentDeviceInfo.name)
            {
                alreadyExists = true;
                break;
            }
        }
        
        if (!alreadyExists)
            devices.add(currentDeviceInfo);
    }
    
    return devices;
}

bool MidiOutputManager::setOutputDevice(const juce::String& deviceIdentifier)
{
    midiOutput.reset();
    isVirtualDevice = false;
    
    if (deviceIdentifier.isEmpty())
        return false;
    
    midiOutput = juce::MidiOutput::openDevice(deviceIdentifier);
    
    if (midiOutput != nullptr)
    {
        currentDeviceInfo = midiOutput->getDeviceInfo();
        midiOutput->startBackgroundThread();
        juce::Logger::writeToLog("Successfully opened MIDI device: " + currentDeviceInfo.name);
        return true;
    }
    
    juce::Logger::writeToLog("Failed to open MIDI device with identifier: " + deviceIdentifier);
    return false;
}

bool MidiOutputManager::createVirtualDevice(const juce::String& deviceName)
{
    midiOutput.reset();
    
    juce::Logger::writeToLog("Attempting to create virtual MIDI device: " + deviceName);
    
    // On macOS, we need proper permissions and entitlements
    // The app should NOT be sandboxed
    midiOutput = juce::MidiOutput::createNewDevice(deviceName);
    
    if (midiOutput != nullptr)
    {
        isVirtualDevice = true;
        currentDeviceInfo = midiOutput->getDeviceInfo();
        midiOutput->startBackgroundThread();
        juce::Logger::writeToLog("Virtual MIDI device created with identifier: " + currentDeviceInfo.identifier);
        
        // Log device info
        juce::Logger::writeToLog("Device name: " + currentDeviceInfo.name);
        juce::Logger::writeToLog("Device identifier: " + currentDeviceInfo.identifier);
        
        return true;
    }
    
    juce::Logger::writeToLog("Failed to create virtual MIDI device");
    juce::Logger::writeToLog("On macOS, this usually happens because of permissions or a sandboxed app.");
    juce::Logger::writeToLog("Make sure your app is not sandboxed and has proper entitlements.");
    
    return false;
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

void MidiOutputManager::sendNoteOff(int channel, int noteNumber)
{
    if (midiOutput != nullptr)
    {
        juce::MidiMessage message = juce::MidiMessage::noteOff(channel, noteNumber);
        midiOutput->sendMessageNow(message);
    }
} 