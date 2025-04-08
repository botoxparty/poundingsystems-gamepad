#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "MidiMappingEditor.h"
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
        setContentOwned(editorComponent.release(), true);
        centreWithSize(800, 600);
        setResizable(true, true);
    }
    
    void closeButtonPressed() override
    {
        // Delete this window when it's closed
        delete this;
    }
    
private:
    // No need to store the editor as a member variable since it's owned by the DialogWindow
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MidiMappingEditorWindow)
}; 