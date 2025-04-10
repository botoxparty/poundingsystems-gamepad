#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "MidiMappingEditor.h"
#include "ModernLookAndFeel.h"
#include "../StandaloneApp.h"

/**
 * A dialog window that contains the MIDI mapping editor
 */
class MidiMappingEditorWindow : public juce::DialogWindow
{
public:
    MidiMappingEditorWindow(StandaloneApp& app)
        : DialogWindow("MIDI Mapping Editor", 
                      juce::Colours::lightgrey,
                      true)
    {
        setUsingNativeTitleBar(true);
        auto editorComponent = std::make_unique<MidiMappingEditor>(app);
        
        // Apply the custom look and feel to the editor component
        editorComponent->setLookAndFeel(&modernLookAndFeel);
        
        setContentOwned(editorComponent.release(), true);
        centreWithSize(300, 400);
        setResizable(false, false);
    }
    
    void closeButtonPressed() override
    {
        // Delete this window when it's closed
        delete this;
    }
    
private:
    // Custom look and feel
    ModernLookAndFeel modernLookAndFeel;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MidiMappingEditorWindow)
}; 