#include "StatusBar.h"

StatusBar::StatusBar(MidiOutputManager& midiOutputManager)
    : midiOutput(midiOutputManager)
{
    // Set up status label with classic Windows style
    statusLabel.setText("No gamepads connected", juce::dontSendNotification);
    statusLabel.setFont(juce::Font("MS Sans Serif", 12.0f, juce::Font::plain));
    statusLabel.setJustificationType(juce::Justification::centredLeft);
    statusLabel.setColour(juce::Label::textColourId, juce::Colours::black);
    addAndMakeVisible(statusLabel);
    
    // Set up MIDI device selector with classic Windows style
    midiDeviceSelector.setTextWhenNothingSelected("Select MIDI Output Device");
    
    // Classic Windows colors for combo box
    midiDeviceSelector.setColour(juce::ComboBox::backgroundColourId, juce::Colours::white);
    midiDeviceSelector.setColour(juce::ComboBox::outlineColourId, juce::Colours::black);
    midiDeviceSelector.setColour(juce::ComboBox::textColourId, juce::Colours::black);
    midiDeviceSelector.setColour(juce::ComboBox::arrowColourId, juce::Colours::black);
    
    // Classic Windows popup menu colors
    midiDeviceSelector.setColour(juce::PopupMenu::backgroundColourId, juce::Colours::white);
    midiDeviceSelector.setColour(juce::PopupMenu::textColourId, juce::Colours::black);
    midiDeviceSelector.setColour(juce::PopupMenu::highlightedBackgroundColourId, juce::Colour(0, 0, 128));  // Classic Windows highlight blue
    midiDeviceSelector.setColour(juce::PopupMenu::highlightedTextColourId, juce::Colours::white);
    
    addAndMakeVisible(midiDeviceSelector);
    refreshMidiDevices();
    
    midiDeviceSelector.onChange = [this] { midiDeviceChanged(); };
    
    setOpaque(true);
}

void StatusBar::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    
    // Classic Windows 98/2000 toolbar style background
    g.fillAll(juce::Colour(192, 192, 192));  // System gray
    
    // Draw beveled edges (classic Windows style)
    g.setColour(juce::Colours::white);
    g.drawLine(0, 0, getWidth() - 1, 0, 1.0f);  // Top
    g.drawLine(0, 0, 0, getHeight() - 1, 1.0f);  // Left
    
    g.setColour(juce::Colours::darkgrey);
    g.drawLine(getWidth() - 1, 0, getWidth() - 1, getHeight() - 1, 1.0f);  // Right
    g.drawLine(0, getHeight() - 1, getWidth() - 1, getHeight() - 1, 1.0f);  // Bottom
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