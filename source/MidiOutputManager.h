#pragma once

#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_core/juce_core.h>
#include <juce_gui_basics/juce_gui_basics.h>

/**
 * Manages MIDI output for the gamepad controller
 * Implemented as a singleton to ensure only one instance exists
 */
class MidiOutputManager
{
public:
    // Get singleton instance
    static MidiOutputManager& getInstance()
    {
        static MidiOutputManager instance;
        return instance;
    }
    
    MidiOutputManager();
    ~MidiOutputManager();
    
    void createInitialVirtualDevice();
    void closeCurrentDevice();
    bool openDevice(const juce::String& identifier);
    bool setOutputDevice(const juce::String& identifier);
    juce::Array<juce::MidiDeviceInfo> getAvailableDevices() const;
    void sendControlChange(int channel, int controller, int value);
    void sendNoteOn(int channel, int noteNumber, float velocity);
    
    // Device management methods
    juce::String getCurrentDeviceIdentifier() const { return currentDeviceInfo.identifier; }
    juce::String getCurrentDeviceName() const { return currentDeviceInfo.name; }
    bool isDeviceOpen() const { return midiOutput != nullptr; }
    
private:
    std::unique_ptr<juce::MidiOutput> midiOutput;
    juce::MidiDeviceInfo currentDeviceInfo;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MidiOutputManager)
}; 