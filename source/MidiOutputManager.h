#pragma once

#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_core/juce_core.h>

/**
 * Manages MIDI output for the gamepad controller
 * Implemented as a singleton to ensure only one instance exists
 */
class MidiOutputManager
{
public:
    // Delete copy constructor and assignment operator
    MidiOutputManager(const MidiOutputManager&) = delete;
    MidiOutputManager& operator=(const MidiOutputManager&) = delete;
    
    // Get singleton instance
    static MidiOutputManager& getInstance()
    {
        static MidiOutputManager instance;
        return instance;
    }
    
    // Get list of available MIDI devices
    juce::Array<juce::MidiDeviceInfo> getAvailableDevices() const;
    
    // Set current MIDI output device
    bool setOutputDevice(const juce::String& deviceIdentifier);
    
    // Create and set a virtual MIDI device if no physical device is available
    bool createVirtualDevice(const juce::String& deviceName);
    
    // Send MIDI control change message
    void sendControlChange(int channel, int controller, int value);
    
    // Send MIDI note messages (velocity should be between 0.0 and 1.0)
    void sendNoteOn(int channel, int noteNumber, float velocity);
    void sendNoteOff(int channel, int noteNumber);
    
    // Get current device info
    const juce::MidiDeviceInfo& getCurrentDeviceInfo() const { return currentDeviceInfo; }
    
private:
    // Private constructor for singleton
    MidiOutputManager();
    ~MidiOutputManager();
    
    void handleDevicesChanged();
    bool waitForDeviceToAppear();
    
    std::unique_ptr<juce::MidiOutput> midiOutput;
    juce::MidiDeviceInfo currentDeviceInfo;
    bool isVirtualDevice = false;
    bool isInitializing = true;
    
    // Keep track of device changes
    juce::MidiDeviceListConnection deviceListConnection;
}; 