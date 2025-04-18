#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_core/juce_core.h>
#include "../StandaloneApp.h"
#include "MidiMappingAccordion.h"

/**
 * A component that displays and allows editing of MIDI mappings
 */
class MidiMappingEditor : public juce::Component,
                         public juce::Button::Listener
{
public:
    MidiMappingEditor(StandaloneApp& app);
    ~MidiMappingEditor() override;
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    
    // Button::Listener implementation
    void buttonClicked(juce::Button* button) override;
    
    // Export and load mappings
    void exportMappings();
    void loadMappings();
    
    // Reset mappings to defaults
    void resetMappings();
    
    // Highlight a control in the editor
    void highlightControl(const juce::String& controlType, int controlIndex);
    
private:
    StandaloneApp& app;
    std::unique_ptr<juce::Viewport> viewport;
    std::unique_ptr<MidiMappingAccordion> accordion;
    
    // Buttons
    juce::TextButton exportButton;
    juce::TextButton loadButton;
    juce::TextButton resetButton;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MidiMappingEditor)
}; 