#include "StatusBar.h"

StatusBar::StatusBar(MidiOutputManager& midiOutputManager)
    : midiOutput(midiOutputManager)
{
    // Set up status label with hypermodern styling
    statusLabel.setText("No gamepads connected", juce::dontSendNotification);
    statusLabel.setFont(juce::Font(16.0f).withStyle(juce::Font::bold));
    statusLabel.setJustificationType(juce::Justification::centredLeft);
    statusLabel.setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(0.95f));
    addAndMakeVisible(statusLabel);
    
    // Set up MIDI device selector with hypermodern styling
    midiDeviceSelector.setTextWhenNothingSelected("Select MIDI Output Device");
    
    // Modern dark theme colors for combo box
    midiDeviceSelector.setColour(juce::ComboBox::backgroundColourId, juce::Colour(35, 35, 40));
    midiDeviceSelector.setColour(juce::ComboBox::outlineColourId, juce::Colour(45, 45, 50).withAlpha(0.5f));
    midiDeviceSelector.setColour(juce::ComboBox::textColourId, juce::Colours::white.withAlpha(0.9f));
    midiDeviceSelector.setColour(juce::ComboBox::arrowColourId, juce::Colour(82, 168, 236));
    
    // Modern popup menu colors
    midiDeviceSelector.setColour(juce::PopupMenu::backgroundColourId, juce::Colour(35, 35, 40));
    midiDeviceSelector.setColour(juce::PopupMenu::textColourId, juce::Colours::white.withAlpha(0.9f));
    midiDeviceSelector.setColour(juce::PopupMenu::highlightedBackgroundColourId, juce::Colour(82, 168, 236));
    midiDeviceSelector.setColour(juce::PopupMenu::highlightedTextColourId, juce::Colours::white);
    
    addAndMakeVisible(midiDeviceSelector);
    refreshMidiDevices();
    
    midiDeviceSelector.onChange = [this] { midiDeviceChanged(); };
    
    setOpaque(true);
}

void StatusBar::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    
    // Create a sleek gradient background
    juce::ColourGradient gradient(
        juce::Colour(28, 28, 32),  // Darker, more sophisticated top color
        0.0f, 0.0f,
        juce::Colour(32, 32, 36),  // Slightly lighter bottom color
        0.0f, (float)getHeight(),
        false);
    g.setGradientFill(gradient);
    g.fillAll();
    
    // Add a subtle glass-like highlight at the top
    juce::ColourGradient highlightGradient(
        juce::Colours::white.withAlpha(0.05f),
        0.0f, 0.0f,
        juce::Colours::transparentWhite,
        0.0f, bounds.getHeight() * 0.5f,
        false);
    g.setGradientFill(highlightGradient);
    g.fillRect(bounds.withHeight(bounds.getHeight() * 0.5f));
    
    // Draw a modern bottom border with gradient
    juce::ColourGradient borderGradient(
        juce::Colour(82, 168, 236).withAlpha(0.2f),  // Accent color
        bounds.getWidth() * 0.5f, bounds.getBottom() - 1,
        juce::Colour(82, 168, 236).withAlpha(0.05f),
        bounds.getRight(), bounds.getBottom() - 1,
        true);  // Radial gradient
    g.setGradientFill(borderGradient);
    g.drawHorizontalLine(getHeight() - 1, 0.0f, (float)getWidth());
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