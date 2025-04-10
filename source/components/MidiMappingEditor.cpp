#include "MidiMappingEditor.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_core/juce_core.h>

MidiMappingEditor::MidiMappingEditor(StandaloneApp& app)
    : app(app)
{
    accordion = std::make_unique<MidiMappingAccordion>(app);
    addAndMakeVisible(accordion.get());
}

MidiMappingEditor::~MidiMappingEditor() = default;

void MidiMappingEditor::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void MidiMappingEditor::resized()
{
    auto bounds = getLocalBounds();
    accordion->setBounds(bounds);
}

void MidiMappingEditor::saveMappings()
{
    accordion->saveMappings();
}

void MidiMappingEditor::loadMappings()
{
    accordion->loadMappings();
}

void MidiMappingEditor::highlightControl(const juce::String& controlType, int controlIndex)
{
    accordion->highlightControl(controlType, controlIndex);
} 