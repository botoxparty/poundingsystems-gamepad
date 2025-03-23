#include "StatusBar.h"

StatusBar::StatusBar(MidiOutputManager& midiOutputManager)
    : midiOutput(midiOutputManager)
{
    // Set up status label with modern style
    statusLabel.setText("No gamepads connected", juce::dontSendNotification);
    statusLabel.setFont(juce::Font(12.0f));
    statusLabel.setJustificationType(juce::Justification::centredLeft);
    statusLabel.setColour(juce::Label::textColourId, juce::Colours::black);
    addAndMakeVisible(statusLabel);
    
    // Set up MIDI device selector with modern style
    midiDeviceSelector.setTextWhenNothingSelected("Select MIDI Output Device");
    
    // Modern colors for combo box
    midiDeviceSelector.setColour(juce::ComboBox::backgroundColourId, juce::Colours::white);
    midiDeviceSelector.setColour(juce::ComboBox::outlineColourId, juce::Colours::darkgrey);
    midiDeviceSelector.setColour(juce::ComboBox::textColourId, juce::Colours::black);
    midiDeviceSelector.setColour(juce::ComboBox::arrowColourId, juce::Colours::black);
    
    // Modern popup menu colors
    midiDeviceSelector.setColour(juce::PopupMenu::backgroundColourId, juce::Colours::white);
    midiDeviceSelector.setColour(juce::PopupMenu::textColourId, juce::Colours::black);
    midiDeviceSelector.setColour(juce::PopupMenu::highlightedBackgroundColourId, juce::Colours::lightgrey);
    midiDeviceSelector.setColour(juce::PopupMenu::highlightedTextColourId, juce::Colours::black);
    
    addAndMakeVisible(midiDeviceSelector);
    refreshMidiDevices();
    
    midiDeviceSelector.onChange = [this] { midiDeviceChanged(); };
    
    setOpaque(true);
}

void StatusBar::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    
    // Light gray background
    g.fillAll(juce::Colour(240, 240, 240));
    
    // Simple border
    g.setColour(juce::Colours::darkgrey);
    g.drawRect(bounds, 1.0f);
}

void StatusBar::resized()
{
    auto area = getLocalBounds().reduced(15, 0);  // Increased horizontal padding
    
    // Layout status bar with modern spacing
    auto statusWidth = 220;  // Slightly wider for better text display
    statusLabel.setBounds(area.removeFromLeft(statusWidth));
    
    // Modern compact selector with perfect centering
    auto selectorHeight = 32;  // Slightly taller for better visibility
    auto selectorArea = area.withSizeKeepingCentre(320, selectorHeight);
    midiDeviceSelector.setBounds(selectorArea);
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