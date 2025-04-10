#include "MidiMappingEditor.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_core/juce_core.h>

MidiMappingEditor::MidiMappingEditor(StandaloneApp& app)
    : app(app)
{
    // Create a viewport to contain the accordion
    viewport = std::make_unique<juce::Viewport>();
    viewport->setScrollBarsShown(true, false); // Show vertical scrollbar, hide horizontal
    viewport->setScrollOnDragEnabled(true);
    
    // Create the accordion
    accordion = std::make_unique<MidiMappingAccordion>(app);
    
    // Add the accordion to the viewport
    viewport->setViewedComponent(accordion.get(), false);
    
    // Add the viewport to this component
    addAndMakeVisible(viewport.get());
    
    // Set up buttons
    saveButton.setButtonText("Save Mappings");
    saveButton.addListener(this);
    addAndMakeVisible(saveButton);
    
    loadButton.setButtonText("Load Mappings");
    loadButton.addListener(this);
    addAndMakeVisible(loadButton);
}

MidiMappingEditor::~MidiMappingEditor() = default;

void MidiMappingEditor::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void MidiMappingEditor::resized()
{
    auto bounds = getLocalBounds();
    
    // Reserve space for the button container at the bottom
    auto buttonArea = bounds.removeFromBottom(40);
    buttonArea.reduce(10, 5);
    
    // Position the buttons
    auto buttonWidth = (buttonArea.getWidth() - 10) / 2;
    saveButton.setBounds(buttonArea.removeFromLeft(buttonWidth));
    buttonArea.removeFromLeft(10);
    loadButton.setBounds(buttonArea);
    
    // Set the viewport to fill the remaining space
    viewport->setBounds(bounds);
    
    // Set a thinner scrollbar
    viewport->setScrollBarThickness(12);
    
    // Set the accordion width to match the viewport width minus the scrollbar width
    // This ensures content fits within the window width
    accordion->setSize(viewport->getWidth() - viewport->getScrollBarThickness(), accordion->getHeight());
    
    // Make sure the viewport's scrollbars are properly configured
    viewport->setScrollOnDragEnabled(true);
}

void MidiMappingEditor::buttonClicked(juce::Button* button)
{
    if (button == &saveButton)
    {
        saveMappings();
    }
    else if (button == &loadButton)
    {
        loadMappings();
    }
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