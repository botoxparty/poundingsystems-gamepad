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
    static MidiMappingEditorWindow* getExistingInstance()
    {
        return existingInstance;
    }

    static MidiMappingEditorWindow* showOrBringToFront(StandaloneApp& app)
    {
        if (existingInstance != nullptr)
        {
            existingInstance->toFront(true);
            return existingInstance;
        }

        auto* window = new MidiMappingEditorWindow(app);
        window->setVisible(true);
        return window;
    }

private:
    MidiMappingEditorWindow(StandaloneApp& app)
        : DialogWindow("MIDI Mapping Editor", 
                      juce::Colours::lightgrey,
                      true)
    {
        jassert(existingInstance == nullptr); // Should never create a second instance
        existingInstance = this;

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
        existingInstance = nullptr;
        delete this;
    }
    
private:
    // Custom look and feel
    ModernLookAndFeel modernLookAndFeel;
    
    // Static instance pointer for singleton pattern
    static MidiMappingEditorWindow* existingInstance;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MidiMappingEditorWindow)
}; 