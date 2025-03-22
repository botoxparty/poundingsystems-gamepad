#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "MidiOutputManager.h"

class StatusBar : public juce::Component
{
public:
    StatusBar(MidiOutputManager& midiOutputManager);
    ~StatusBar() override = default;
    
    void resized() override;
    void setNumGamepads(int numGamepads);
    
private:
    void refreshMidiDevices();
    void midiDeviceChanged();
    
    juce::Label statusLabel;
    juce::ComboBox midiDeviceSelector;
    MidiOutputManager& midiOutput;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StatusBar)
}; 