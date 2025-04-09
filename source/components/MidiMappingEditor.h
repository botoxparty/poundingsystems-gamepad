#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_core/juce_core.h>
#include "../StandaloneApp.h"

/**
 * A component that displays and allows editing of MIDI mappings
 */
class MidiMappingEditor : public juce::Component,
                         public juce::TableListBoxModel,
                         public juce::Button::Listener,
                         public juce::TextEditor::Listener
{
public:
    MidiMappingEditor(StandaloneApp& app);
    ~MidiMappingEditor() override;
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    
    // TableListBoxModel implementation
    int getNumRows() override;
    void paintRowBackground(juce::Graphics& g, int rowNumber, int width, int height, bool rowIsSelected) override;
    void paintCell(juce::Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected) override;
    juce::Component* refreshComponentForCell(int rowNumber, int columnId, bool isRowSelected, juce::Component* existingComponentToUpdate) override;
    
    // Button::Listener implementation
    void buttonClicked(juce::Button* button) override;
    
    // TextEditor::Listener implementation
    void textEditorTextChanged(juce::TextEditor& editor) override;
    void textEditorReturnKeyPressed(juce::TextEditor& editor) override;
    void textEditorEscapeKeyPressed(juce::TextEditor& editor) override;
    void textEditorFocusLost(juce::TextEditor& editor) override;
    
    // Save and load mappings
    void saveMappings();
    void loadMappings();
    
    // Highlight a control in the editor
    void highlightControl(const juce::String& controlType, int controlIndex);
    
private:
    struct MappingData
    {
        juce::String controlName;
        juce::String controlType;
        int controlIndex;
        std::vector<StandaloneApp::MidiMapping> mappings;
    };
    
    void updateMappingData();
    void addMapping(int rowIndex);
    void removeMapping(int rowIndex);
    juce::String getControlName(const juce::String& controlType, int index) const;
    void updateAppMappings();
    
    StandaloneApp& app;
    juce::TableListBox mappingTable;
    juce::TextButton addMappingButton;
    juce::TextButton removeMappingButton;
    juce::TextButton saveButton;
    juce::TextButton loadButton;
    
    std::vector<MappingData> mappingData;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MidiMappingEditor)
}; 