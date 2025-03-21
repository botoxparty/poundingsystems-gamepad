#pragma once

#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_core/juce_core.h>

/**
 * Manages MIDI output for the gamepad controller
 */
class MidiOutputManager
{
public:
    MidiOutputManager();
    ~MidiOutputManager();
    
    // Get list of available MIDI output devices
    juce::Array<juce::MidiDeviceInfo> getAvailableDevices() const;
    
    // Set current MIDI output device
    bool setOutputDevice(const juce::String& deviceIdentifier);
    
    // Send MIDI control change message
    void sendControlChange(int channel, int controller, int value);
    
    // Send MIDI note messages (velocity should be between 0.0 and 1.0)
    void sendNoteOn(int channel, int noteNumber, float velocity);
    void sendNoteOff(int channel, int noteNumber);
    
    // Get current device info
    const juce::MidiDeviceInfo& getCurrentDeviceInfo() const { return currentDeviceInfo; }
    
private:
    std::unique_ptr<juce::MidiOutput> midiOutput;
    juce::MidiDeviceInfo currentDeviceInfo;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MidiOutputManager)
}; 