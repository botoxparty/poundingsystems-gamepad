#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_core/juce_core.h>
#include "../StandaloneApp.h"

/**
 * A component that displays and allows editing of MIDI mappings using an accordion-style UI
 */
class MidiMappingAccordion : public juce::Component,
                            public juce::Button::Listener
{
public:
    MidiMappingAccordion(StandaloneApp& app);
    ~MidiMappingAccordion() override;
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    
    // Button::Listener implementation
    void buttonClicked(juce::Button* button) override;
    
    // Save and load mappings
    void saveMappings();
    void loadMappings();
    
    // Highlight a control in the editor
    void highlightControl(const juce::String& controlType, int controlIndex);
    
private:
    // Represents a single control with its mappings
    class ControlItem : public juce::Component,
                        public juce::Button::Listener
    {
    public:
        class MappingsList : public juce::Component
        {
        public:
            MappingsList(const std::vector<StandaloneApp::MidiMapping>& mappings);
            void paint(juce::Graphics& g) override;
            void resized() override;
            void updateMappings(const std::vector<StandaloneApp::MidiMapping>& newMappings);
            
        private:
            std::vector<StandaloneApp::MidiMapping> mappings;
        };
        
        // Add a custom header component class
        class HeaderComponent : public juce::Component
        {
        public:
            HeaderComponent(ControlItem& owner);
            void paint(juce::Graphics& g) override;
            void mouseDown(const juce::MouseEvent&) override;
            
        private:
            ControlItem& owner;
        };
        
        ControlItem(const juce::String& name, 
                   const juce::String& type, 
                   int index, 
                   const std::vector<StandaloneApp::MidiMapping>& mappings,
                   MidiMappingAccordion& parent);
        
        void paint(juce::Graphics& g) override;
        void resized() override;
        void buttonClicked(juce::Button* button) override;
        
        void setExpanded(bool shouldBeExpanded);
        void setHighlighted(bool shouldBeHighlighted);
        void updateMappings(const std::vector<StandaloneApp::MidiMapping>& newMappings);
        
        bool isExpanded() const { return expanded; }
        const juce::String& getControlType() const { return controlType; }
        int getControlIndex() const { return controlIndex; }
        const std::vector<StandaloneApp::MidiMapping>& getMappings() const { return mappings; }
        const juce::String& getName() const { return controlName; }
        
    private:
        juce::String controlName;
        juce::String controlType;
        int controlIndex;
        std::vector<StandaloneApp::MidiMapping> mappings;
        MidiMappingAccordion& parent;
        
        std::unique_ptr<HeaderComponent> headerComponent;
        juce::TextButton expandButton;
        juce::TextButton addMappingButton;
        juce::TextButton removeMappingButton;
        std::unique_ptr<MappingsList> mappingsList;
        
        bool expanded = false;
        bool highlighted = false;
    };
    
    void updateMappingData();
    void addMapping(ControlItem* controlItem);
    void removeMapping(ControlItem* controlItem);
    void updateAppMappings();
    juce::String getControlName(const juce::String& controlType, int index) const;
    juce::String getMidiNoteName(int midiNoteNumber);
    
    StandaloneApp& app;
    std::vector<std::unique_ptr<ControlItem>> controlItems;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MidiMappingAccordion)
}; 