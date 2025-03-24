#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "GamepadManager.h"
#include "MidiOutputManager.h"
#include "components/ModernGamepadComponent.h"
#include "BinaryData.h"

class StandaloneApp : public juce::Component,
                      private juce::Timer
{
public:
    StandaloneApp();
    ~StandaloneApp() override;
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    
private:
    // About window component
    class AboutWindow : public juce::DialogWindow
    {
    public:
        AboutWindow() 
            : DialogWindow("About Pounding Systems Gamepad", 
                          juce::Colours::darkgrey,
                          true) 
        {
            setUsingNativeTitleBar(true);
            auto content = std::make_unique<juce::Component>();
            content->setSize(300, 200);
            
            auto* label = new juce::Label();
            label->setText("Pounding Systems Gamepad\n\n"
                         "Version 1.0.0\n\n"
                         "2025 Pounding Systems", 
                         juce::dontSendNotification);
            label->setJustificationType(juce::Justification::centred);
            label->setBounds(10, 10, 280, 140);
            content->addAndMakeVisible(label);
            
            auto* websiteLink = new juce::HyperlinkButton("https://pounding.systems",
                                                         juce::URL("https://pounding.systems"));
            websiteLink->setJustificationType(juce::Justification::centred);
            websiteLink->setFont(juce::Font(14.0f), false);
            websiteLink->setBounds(10, 150, 280, 30);
            content->addAndMakeVisible(websiteLink);
            
            setContentOwned(content.release(), true);
            centreWithSize(getWidth(), getHeight());
            setResizable(false, false);
        }
        
        void closeButtonPressed() override
        {
            setVisible(false);
        }
    };
    
    void handleLogoClick();
    void timerCallback() override;
    void handleGamepadStateChange();
    void setupMidiMappings();
    void mouseUp(const juce::MouseEvent& event) override;
    
    // State tracking for single gamepad
    struct GamepadState {
        float axes[GamepadManager::MAX_AXES] = {};
        bool buttons[GamepadManager::MAX_BUTTONS] = {};
        bool connected = false;
    };
    GamepadState previousGamepadState;
    
    // UI Components
    std::unique_ptr<ModernGamepadComponent> gamepadComponent;
    juce::ImageComponent logoComponent;
    
    // Managers
    GamepadManager gamepadManager;
    
    // MIDI mapping configuration
    struct MidiMapping {
        int channel;
        int ccNumber;
        float minValue;
        float maxValue;
        bool isButton;
    };
    
    std::array<MidiMapping, GamepadManager::MAX_AXES> axisMappings;
    std::array<MidiMapping, GamepadManager::MAX_BUTTONS> buttonMappings;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StandaloneApp)
}; 