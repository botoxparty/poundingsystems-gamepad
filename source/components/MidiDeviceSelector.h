#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../MidiOutputManager.h"

class MidiDeviceSelector : public juce::Component,
                          public juce::ComboBox::Listener
{
public:
    MidiDeviceSelector()
    {
        // Setup label
        addAndMakeVisible(label);
        label.setText("MIDI Output:", juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centredRight);
        label.setColour(juce::Label::textColourId, juce::Colours::black);
        label.setFont(juce::Font(juce::Font::getDefaultSansSerifFontName(), 14.0f, juce::Font::plain));
        
        // Setup combo box
        addAndMakeVisible(deviceSelector);
        deviceSelector.setJustificationType(juce::Justification::centredLeft);
        deviceSelector.setTextWhenNothingSelected("Select MIDI Device");
        deviceSelector.setColour(juce::ComboBox::backgroundColourId, juce::Colours::white);
        deviceSelector.setColour(juce::ComboBox::textColourId, juce::Colours::black);
        deviceSelector.setColour(juce::ComboBox::outlineColourId, juce::Colours::darkgrey);
        deviceSelector.setColour(juce::ComboBox::arrowColourId, juce::Colours::black);
        deviceSelector.setColour(juce::ComboBox::focusedOutlineColourId, juce::Colours::blue);
        deviceSelector.setColour(juce::ComboBox::buttonColourId, juce::Colours::white);
        deviceSelector.setColour(juce::ComboBox::textColourId, juce::Colours::black);
        deviceSelector.addListener(this);
        
        refreshDeviceList();
    }
    
    void visibilityChanged() override
    {
        if (isVisible())
        {
            refreshDeviceList();
        }
    }
    
    void refreshDeviceList()
    {
        deviceSelector.clear();
        auto devices = MidiOutputManager::getInstance().getAvailableDevices();
        
        juce::Logger::writeToLog("MidiDeviceSelector - Available MIDI devices:");
        for (const auto& device : devices)
        {
            juce::Logger::writeToLog(" - " + device.name + " (" + device.identifier + ")");
            deviceSelector.addItem(device.name, devices.indexOf(device) + 1);
        }
        
        // Select current device
        auto currentDevice = MidiOutputManager::getInstance().getCurrentDeviceIdentifier();
        if (!currentDevice.isEmpty())
        {
            juce::Logger::writeToLog("MidiDeviceSelector - Current device: " + currentDevice);
            for (int i = 0; i < devices.size(); ++i)
            {
                if (devices[i].identifier == currentDevice)
                {
                    deviceSelector.setSelectedId(i + 1);
                    break;
                }
            }
        }
    }
    
    void resized() override
    {
        auto bounds = getLocalBounds();
        
        // Label takes 100px width
        auto labelWidth = 100;
        label.setBounds(bounds.removeFromLeft(labelWidth));
        
        // Combo box takes remaining space
        deviceSelector.setBounds(bounds.reduced(5, 0));
    }
    
    void comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged) override
    {
        if (comboBoxThatHasChanged == &deviceSelector)
        {
            auto selectedIndex = deviceSelector.getSelectedId() - 1;
            auto devices = MidiOutputManager::getInstance().getAvailableDevices();
            
            if (selectedIndex >= 0 && selectedIndex < devices.size())
            {
                juce::Logger::writeToLog("MidiDeviceSelector - Selected device: " + devices[selectedIndex].name + 
                                        " (" + devices[selectedIndex].identifier + ")");
                MidiOutputManager::getInstance().setOutputDevice(devices[selectedIndex].identifier);
            }
        }
    }
    
private:
    class RefreshingComboBox : public juce::ComboBox
    {
    public:
        RefreshingComboBox(MidiDeviceSelector& owner) : owner(owner) {}
        
        void mouseDown(const juce::MouseEvent& e) override
        {
            owner.refreshDeviceList();
            juce::ComboBox::mouseDown(e);
        }
        
    private:
        MidiDeviceSelector& owner;
    };

    juce::Label label;
    RefreshingComboBox deviceSelector{*this};
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MidiDeviceSelector)
}; 