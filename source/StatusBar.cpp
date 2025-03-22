#include "StatusBar.h"

StatusBar::StatusBar(MidiOutputManager& midiOutputManager)
    : midiOutput(midiOutputManager)
{
    // Set up status label
    statusLabel.setText("No gamepads connected", juce::dontSendNotification);
    statusLabel.setFont(juce::Font(14.0f));
    statusLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(statusLabel);
    
    // Set up MIDI device selector
    midiDeviceSelector.setTextWhenNothingSelected("Select MIDI Output Device");
    addAndMakeVisible(midiDeviceSelector);
    refreshMidiDevices();
    
    midiDeviceSelector.onChange = [this] { midiDeviceChanged(); };
}

void StatusBar::resized()
{
    auto area = getLocalBounds();
    
    // Layout status bar: connection status on left, MIDI selector on right
    auto statusWidth = 200;
    statusLabel.setBounds(area.removeFromLeft(statusWidth));
    midiDeviceSelector.setBounds(area.withSizeKeepingCentre(300, getHeight()));
}

void StatusBar::setNumGamepads(int numGamepads)
{
    statusLabel.setText(numGamepads > 0 
        ? juce::String(numGamepads) + " gamepad" + (numGamepads > 1 ? "s" : "") + " connected"
        : "No gamepads connected",
        juce::dontSendNotification);
}

void StatusBar::refreshMidiDevices()
{
    midiDeviceSelector.clear();
    
    auto devices = midiOutput.getAvailableDevices();
    int index = 1;
    
    for (const auto& device : devices)
    {
        midiDeviceSelector.addItem(device.name, index++);
    }
}

void StatusBar::midiDeviceChanged()
{
    if (midiDeviceSelector.getSelectedId() > 0)
    {
        auto devices = midiOutput.getAvailableDevices();
        auto selectedDevice = devices[midiDeviceSelector.getSelectedItemIndex()];
        midiOutput.setOutputDevice(selectedDevice.identifier);
    }
} 