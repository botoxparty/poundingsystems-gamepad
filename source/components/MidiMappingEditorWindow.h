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
        editor = std::make_unique<MidiMappingEditor>(app);
        setContentOwned(editor.release(), true);
        centreWithSize(800, 600);
        setResizable(true, true);
    }
    
    void closeButtonPressed() override
    {
        setVisible(false);
    }
    
private:
    std::unique_ptr<MidiMappingEditor> editor;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MidiMappingEditorWindow)
}; 