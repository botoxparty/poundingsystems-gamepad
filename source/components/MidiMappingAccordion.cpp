#include "MidiMappingAccordion.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_core/juce_core.h>

// Implementation of ControlItem::MappingsList
MidiMappingAccordion::ControlItem::MappingsList::MappingsList(const std::vector<StandaloneApp::MidiMapping>& mappings)
    : mappings(mappings)
{
}

void MidiMappingAccordion::ControlItem::MappingsList::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::white);
    
    if (mappings.empty())
    {
        g.setColour(juce::Colours::darkgrey);
        g.drawText("No mappings", getLocalBounds(), juce::Justification::centred, true);
        return;
    }
    
    g.setColour(juce::Colours::black);
    g.setFont(14.0f);
    
    int y = 5;
    for (size_t i = 0; i < mappings.size(); ++i)
    {
        const auto& mapping = mappings[i];
        juce::String mappingText;
        
        if (mapping.type == StandaloneApp::MidiMapping::Type::ControlChange)
        {
            mappingText = juce::String("Ch:") + juce::String(mapping.channel) +
                         " CC:" + juce::String(mapping.ccNumber) +
                         " [" + juce::String(mapping.minValue) + "-" + juce::String(mapping.maxValue) + "]";
        }
        else // Note
        {
            mappingText = juce::String("Ch:") + juce::String(mapping.channel) +
                         " Note:" + juce::String(mapping.noteNumber) +
                         " [" + juce::String(mapping.minValue) + "-" + juce::String(mapping.maxValue) + "]";
        }
        
        // Draw a subtle background for each mapping item
        g.setColour(juce::Colours::lightgrey.withAlpha(0.3f));
        g.fillRect(juce::Rectangle<float>(5.0f, y, getWidth() - 10, 22));
        
        g.setColour(juce::Colours::black);
        g.drawText(mappingText, 10, y, getWidth() - 20, 22, juce::Justification::centredLeft, true);
        y += 30;
    }
}

void MidiMappingAccordion::ControlItem::MappingsList::resized()
{
    // Nothing to do here
}

void MidiMappingAccordion::ControlItem::MappingsList::updateMappings(const std::vector<StandaloneApp::MidiMapping>& newMappings)
{
    mappings = newMappings;
    repaint();
}

// Implementation of HeaderComponent
MidiMappingAccordion::ControlItem::HeaderComponent::HeaderComponent(ControlItem& owner)
    : owner(owner)
{
    // Make the header component mouse clickable
    setMouseClickGrabsKeyboardFocus(true);
}

void MidiMappingAccordion::ControlItem::HeaderComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    
    // Draw header background with a subtle gradient
    juce::ColourGradient gradient(
        juce::Colours::lightgrey,
        0.0f, 0.0f,
        juce::Colours::lightgrey.brighter(0.1f),
        0.0f, (float)bounds.getHeight(),
        false);
    g.setGradientFill(gradient);
    g.fillRect(bounds.toFloat());
    
    // Draw a subtle border
    g.setColour(juce::Colours::grey.withAlpha(0.5f));
    g.drawRect(bounds.toFloat(), 1.0f);
    
    // Draw control name
    g.setColour(juce::Colours::black);
    g.setFont(16.0f);
    g.drawText(owner.controlName, bounds.reduced(10).withTrimmedRight(30), juce::Justification::centredLeft, true);
    
    // Draw summary of mappings if not expanded
    if (!owner.expanded && !owner.mappings.empty())
    {
        g.setFont(14.0f);
        g.setColour(juce::Colours::darkgrey);
        
        juce::String summaryText = juce::String(owner.mappings.size()) + " mapping" + 
                                  (owner.mappings.size() > 1 ? "s" : "");
        
        g.drawText(summaryText, bounds.reduced(10).withTrimmedRight(30), juce::Justification::centredRight, true);
    }
}

void MidiMappingAccordion::ControlItem::HeaderComponent::mouseDown(const juce::MouseEvent&)
{
    // Toggle expanded state when header is clicked
    owner.setExpanded(!owner.expanded);
}

// Implementation of ControlItem
MidiMappingAccordion::ControlItem::ControlItem(const juce::String& name, 
                                             const juce::String& type, 
                                             int index, 
                                             const std::vector<StandaloneApp::MidiMapping>& mappings,
                                             MidiMappingAccordion& parent)
    : controlName(name),
      controlType(type),
      controlIndex(index),
      mappings(mappings),
      parent(parent)
{
    // Create header component
    headerComponent = std::make_unique<HeaderComponent>(*this);
    headerComponent->setSize(100, 30);
    addAndMakeVisible(headerComponent.get());
    
    // Set up expand button in header with improved styling
    expandButton.setButtonText("+");
    expandButton.addListener(this);
    expandButton.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    // expandButton.setColour(juce::TextButton::textColourId, juce::Colours::black);
    headerComponent->addAndMakeVisible(expandButton);
    
    // Set up add/remove buttons with improved styling
    addMappingButton.setButtonText("Add Mapping");
    addMappingButton.addListener(this);
    addMappingButton.setColour(juce::TextButton::buttonColourId, juce::Colours::lightblue);
    // addMappingButton.setColour(juce::TextButton::textColourId, juce::Colours::black);
    addAndMakeVisible(addMappingButton);
    
    removeMappingButton.setButtonText("Remove Mapping");
    removeMappingButton.addListener(this);
    removeMappingButton.setColour(juce::TextButton::buttonColourId, juce::Colours::lightcoral);
    // removeMappingButton.setColour(juce::TextButton::textColourId, juce::Colours::black);
    addAndMakeVisible(removeMappingButton);
    
    // Set up mappings list
    mappingsList = std::make_unique<MappingsList>(mappings);
    addAndMakeVisible(mappingsList.get());
}

void MidiMappingAccordion::ControlItem::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    
    // Draw background
    if (highlighted)
    {
        // Flash effect for highlighted controls
        auto currentTime = juce::Time::getMillisecondCounter();
        bool isFlashing = (currentTime / 100) % 2 == 0; // Flash every 100ms
        
        if (isFlashing)
            g.fillAll(juce::Colours::orange.withAlpha(0.3f));
        else
            g.fillAll(juce::Colours::lightblue.withAlpha(0.3f));
    }
    else
    {
        g.fillAll(juce::Colours::white);
    }
    
    // Draw a subtle border around the entire control item
    g.setColour(juce::Colours::grey.withAlpha(0.3f));
    g.drawRect(bounds.toFloat(), 1.0f);
}

void MidiMappingAccordion::ControlItem::resized()
{
    auto bounds = getLocalBounds();
    
    // Define header height - reduced from 40 to 30
    const int headerHeight = 30;
    
    // Position header component
    headerComponent->setBounds(bounds.removeFromTop(headerHeight));
    
    // Position expand button in header
    expandButton.setBounds(headerComponent->getWidth() - 25, 2, 20, 26);
    
    // Position add/remove buttons if expanded
    if (expanded)
    {
        // Position buttons in the body area
        auto buttonArea = bounds.removeFromBottom(30).reduced(5);
        addMappingButton.setBounds(buttonArea.removeFromLeft(100));
        buttonArea.removeFromLeft(10);
        removeMappingButton.setBounds(buttonArea.removeFromLeft(100));
        
        // Position mappings list in the body area
        mappingsList->setBounds(bounds.reduced(5));
    }
    else
    {
        // Ensure mappings list is hidden when collapsed
        if (mappingsList != nullptr)
            mappingsList->setBounds(0, 0, 0, 0);
    }
}

void MidiMappingAccordion::ControlItem::buttonClicked(juce::Button* button)
{
    if (button == &expandButton)
    {
        setExpanded(!expanded);
    }
    else if (button == &addMappingButton)
    {
        parent.addMapping(this);
    }
    else if (button == &removeMappingButton)
    {
        parent.removeMapping(this);
    }
}

void MidiMappingAccordion::ControlItem::updateMappings(const std::vector<StandaloneApp::MidiMapping>& newMappings)
{
    mappings = newMappings;
    if (mappingsList != nullptr)
        mappingsList->updateMappings(mappings);
    repaint();
}

void MidiMappingAccordion::ControlItem::setExpanded(bool shouldBeExpanded)
{
    if (expanded != shouldBeExpanded)
    {
        expanded = shouldBeExpanded;
        expandButton.setButtonText(expanded ? "-" : "+");
        
        // Ensure proper visibility of components
        if (mappingsList != nullptr)
            mappingsList->setVisible(expanded);
        
        addMappingButton.setVisible(expanded);
        removeMappingButton.setVisible(expanded);
        
        // Repaint the header component to update the UI
        if (headerComponent != nullptr)
            headerComponent->repaint();
        
        // Store the current viewport position before resizing
        auto* viewport = findParentComponentOfClass<juce::Viewport>();
        int scrollY = 0;
        if (viewport != nullptr)
            scrollY = viewport->getViewPositionY();
        
        resized();
        parent.resized();
        
        // Restore the viewport position after resizing
        if (viewport != nullptr)
            viewport->setViewPosition(0, scrollY);
    }
}

void MidiMappingAccordion::ControlItem::setHighlighted(bool shouldBeHighlighted)
{
    if (highlighted != shouldBeHighlighted)
    {
        highlighted = shouldBeHighlighted;
        repaint();
    }
}

// Implementation of MidiMappingAccordion
MidiMappingAccordion::MidiMappingAccordion(StandaloneApp& app)
    : app(app)
{
    // Set a fixed width for the component
    setSize(400, 500);
    
    // Set up viewport
    viewport.setViewedComponent(&viewportContent);
    viewport.setScrollBarsShown(true, true);
    viewport.setScrollOnDragEnabled(true);
    viewport.setScrollBarThickness(12);
    addAndMakeVisible(viewport);
    
    // Initialize mapping data
    updateMappingData();
}

MidiMappingAccordion::~MidiMappingAccordion()
{
    viewport.setViewedComponent(nullptr);
}

void MidiMappingAccordion::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::white);
    
    // Draw a subtle border around the entire component
    g.setColour(juce::Colours::grey.withAlpha(0.5f));
    g.drawRect(getLocalBounds().toFloat(), 1.0f);
    
    // Set a fixed width for the component
    setSize(400, getHeight());
}

void MidiMappingAccordion::resized()
{
    auto area = getLocalBounds();
    
    // Position viewport
    viewport.setBounds(area);
    
    // Get the scrollbar width
    int scrollbarWidth = viewport.getScrollBarThickness();
    
    // Position control items with no padding between them
    int y = 0;
    int maxWidth = 0;
    
    for (auto& item : controlItems)
    {
        // Calculate height based on expanded state
        int itemHeight = item->isExpanded() ? 200 : 30; // 30px for header, 170px for body when expanded
        
        // Make items take the full width of the viewport minus the scrollbar width
        item->setBounds(0, y, viewport.getWidth() - scrollbarWidth, itemHeight);
        y += itemHeight; // No spacing between items
        maxWidth = juce::jmax(maxWidth, item->getWidth());
    }
    
    // Make the content take the full width of the viewport minus the scrollbar width
    viewportContent.setBounds(0, 0, viewport.getWidth() - scrollbarWidth, y);
}

void MidiMappingAccordion::buttonClicked(juce::Button* button)
{
    // Button click handling for other buttons remains
}

void MidiMappingAccordion::updateMappingData()
{
    controlItems.clear();
    
    // Add axis mappings
    for (int i = 0; i < GamepadManager::MAX_AXES; ++i)
    {
        auto name = getControlName("Axis", i);
        auto item = std::make_unique<ControlItem>(name, "Axis", i, app.axisMappings[static_cast<size_t>(i)], *this);
        controlItems.push_back(std::move(item));
    }
    
    // Add button mappings
    for (int i = 0; i < GamepadManager::MAX_BUTTONS; ++i)
    {
        auto name = getControlName("Button", i);
        auto item = std::make_unique<ControlItem>(name, "Button", i, app.buttonMappings[static_cast<size_t>(i)], *this);
        controlItems.push_back(std::move(item));
    }
    
    // Add gyroscope mappings
    for (int i = 0; i < 3; ++i)
    {
        auto name = getControlName("Gyro", i);
        auto item = std::make_unique<ControlItem>(name, "Gyro", i, app.gyroMappings[static_cast<size_t>(i)], *this);
        controlItems.push_back(std::move(item));
    }
    
    // Add accelerometer mappings
    for (int i = 0; i < 3; ++i)
    {
        auto name = getControlName("Accel", i);
        auto item = std::make_unique<ControlItem>(name, "Accel", i, app.accelerometerMappings[static_cast<size_t>(i)], *this);
        controlItems.push_back(std::move(item));
    }
    
    // Add all items to the viewport content
    for (auto& item : controlItems)
    {
        viewportContent.addAndMakeVisible(item.get());
    }
    
    resized();
}

void MidiMappingAccordion::addMapping(ControlItem* controlItem)
{
    // Create a dialog to get mapping details
    juce::DialogWindow::LaunchOptions options;
    auto* content = new juce::Component();
    content->setSize(300, 250);
    
    auto* channelLabel = new juce::Label("channel", "MIDI Channel:");
    channelLabel->setColour(juce::Label::textColourId, juce::Colours::black);
    auto* channelEditor = new juce::TextEditor();
    channelEditor->setText("1");
    channelEditor->setColour(juce::TextEditor::textColourId, juce::Colours::black);
    channelEditor->setColour(juce::TextEditor::backgroundColourId, juce::Colours::white);
    
    auto* typeLabel = new juce::Label("type", "MIDI Type:");
    typeLabel->setColour(juce::Label::textColourId, juce::Colours::black);
    auto* typeComboBox = new juce::ComboBox("typeComboBox");
    typeComboBox->addItem("Control Change (CC)", 1);
    typeComboBox->addItem("Note", 2);
    typeComboBox->setSelectedId(1);
    
    auto* ccLabel = new juce::Label("cc", "CC Number:");
    ccLabel->setColour(juce::Label::textColourId, juce::Colours::black);
    auto* ccEditor = new juce::TextEditor();
    ccEditor->setText("1");
    ccEditor->setColour(juce::TextEditor::textColourId, juce::Colours::black);
    ccEditor->setColour(juce::TextEditor::backgroundColourId, juce::Colours::white);
    
    auto* noteLabel = new juce::Label("note", "Note Number:");
    noteLabel->setColour(juce::Label::textColourId, juce::Colours::black);
    
    // Replace the text editor with a ComboBox for note selection
    auto* noteComboBox = new juce::ComboBox("noteComboBox");
    noteComboBox->setColour(juce::ComboBox::textColourId, juce::Colours::black);
    noteComboBox->setColour(juce::ComboBox::backgroundColourId, juce::Colours::white);
    
    // Configure the ComboBox to show a scrollable list
    noteComboBox->setScrollWheelEnabled(true);
    
    // Set a reasonable height for the ComboBox
    noteComboBox->setSize(noteComboBox->getWidth(), 24);
    
    // Create a custom LookAndFeel to limit the dropdown height
    class LimitedHeightComboBoxLookAndFeel : public juce::LookAndFeel_V4
    {
    public:
        juce::PopupMenu::Options getOptionsForComboBoxPopupMenu(juce::ComboBox& box, juce::Label& label) override
        {
            auto options = juce::LookAndFeel_V4::getOptionsForComboBoxPopupMenu(box, label);
            
            // Calculate a reasonable height for the dropdown (12 items)
            const int itemHeight = label.getHeight();
            const int maxHeight = itemHeight * 12;
            
            // Set the maximum height for the popup menu
            options = options.withMaximumNumColumns(1);
            
            return options;
        }
    };
    
    // Apply the custom LookAndFeel to the ComboBox
    static LimitedHeightComboBoxLookAndFeel limitedHeightLookAndFeel;
    noteComboBox->setLookAndFeel(&limitedHeightLookAndFeel);
    
    // Add MIDI notes to the combo box (0-127)
    for (int i = 0; i < 128; ++i)
    {
        juce::String noteName = getMidiNoteName(i);
        noteComboBox->addItem(juce::String(i) + " - " + noteName, i + 1);
    }
    noteComboBox->setSelectedId(61); // Middle C (MIDI note 60) + 1 for 1-based indexing
    noteComboBox->setEnabled(false);  // Initially disabled
    noteComboBox->setVisible(false);  // Initially hidden
    noteLabel->setVisible(false);   // Initially hidden
    
    auto* minLabel = new juce::Label("min", "Min Value:");
    minLabel->setColour(juce::Label::textColourId, juce::Colours::black);
    auto* minEditor = new juce::TextEditor();
    minEditor->setText("0");
    minEditor->setColour(juce::TextEditor::textColourId, juce::Colours::black);
    minEditor->setColour(juce::TextEditor::backgroundColourId, juce::Colours::white);
    
    auto* maxLabel = new juce::Label("max", "Max Value:");
    maxLabel->setColour(juce::Label::textColourId, juce::Colours::black);
    auto* maxEditor = new juce::TextEditor();
    maxEditor->setText("127");
    maxEditor->setColour(juce::TextEditor::textColourId, juce::Colours::black);
    maxEditor->setColour(juce::TextEditor::backgroundColourId, juce::Colours::white);
    
    auto* okButton = new juce::TextButton("OK");
    auto* cancelButton = new juce::TextButton("Cancel");
    
    content->addAndMakeVisible(channelLabel);
    content->addAndMakeVisible(channelEditor);
    content->addAndMakeVisible(typeLabel);
    content->addAndMakeVisible(typeComboBox);
    content->addAndMakeVisible(ccLabel);
    content->addAndMakeVisible(ccEditor);
    content->addAndMakeVisible(noteLabel);
    content->addAndMakeVisible(noteComboBox);
    content->addAndMakeVisible(minLabel);
    content->addAndMakeVisible(minEditor);
    content->addAndMakeVisible(maxLabel);
    content->addAndMakeVisible(maxEditor);
    content->addAndMakeVisible(okButton);
    content->addAndMakeVisible(cancelButton);
    
    // Layout components
    auto bounds = content->getLocalBounds().reduced(10);
    channelLabel->setBounds(bounds.removeFromTop(20));
    channelEditor->setBounds(bounds.removeFromTop(20));
    bounds.removeFromTop(10);
    
    typeLabel->setBounds(bounds.removeFromTop(20));
    typeComboBox->setBounds(bounds.removeFromTop(20));
    bounds.removeFromTop(10);
    
    // Create a function to update the layout when visibility changes
    auto updateLayout = [=]() {
        auto layoutBounds = content->getLocalBounds().reduced(10);
        
        // Position the first components
        layoutBounds.removeFromTop(20); // channelLabel
        layoutBounds.removeFromTop(20); // channelEditor
        layoutBounds.removeFromTop(10);
        
        layoutBounds.removeFromTop(20); // typeLabel
        layoutBounds.removeFromTop(20); // typeComboBox
        layoutBounds.removeFromTop(10);
        
        // Position CC or Note components based on visibility
        if (ccLabel->isVisible()) {
            ccLabel->setBounds(layoutBounds.removeFromTop(20));
            ccEditor->setBounds(layoutBounds.removeFromTop(20));
            layoutBounds.removeFromTop(10);
        }
        
        if (noteLabel->isVisible()) {
            noteLabel->setBounds(layoutBounds.removeFromTop(20));
            noteComboBox->setBounds(layoutBounds.removeFromTop(20));
            layoutBounds.removeFromTop(10);
        }
        
        // Position the remaining components
        auto minMaxRow = layoutBounds.removeFromTop(20);
        minLabel->setBounds(minMaxRow.removeFromLeft(70));
        minEditor->setBounds(minMaxRow.removeFromLeft(70));
        minMaxRow.removeFromLeft(5); // Add some spacing between min and max
        maxLabel->setBounds(minMaxRow.removeFromLeft(70));
        maxEditor->setBounds(minMaxRow.removeFromLeft(70));
        layoutBounds.removeFromTop(10);
        
        auto buttonArea = layoutBounds.removeFromBottom(30);
        okButton->setBounds(buttonArea.removeFromLeft(100));
        buttonArea.removeFromLeft(10);
        cancelButton->setBounds(buttonArea.removeFromLeft(100));
    };
    
    // Initial layout
    updateLayout();
    
    // Handle type selection change
    typeComboBox->onChange = [ccEditor, noteComboBox, ccLabel, noteLabel, typeComboBox, updateLayout]() {
        bool isCC = typeComboBox->getSelectedId() == 1;
        ccEditor->setEnabled(isCC);
        ccEditor->setVisible(isCC);
        ccLabel->setVisible(isCC);
        noteComboBox->setEnabled(!isCC);
        noteComboBox->setVisible(!isCC);
        noteLabel->setVisible(!isCC);
        
        // Update layout after changing visibility
        updateLayout();
    };
    
    // Handle button clicks
    okButton->onClick = [this, content, channelEditor, typeComboBox, ccEditor, noteComboBox, minEditor, maxEditor, controlItem]()
    {
        StandaloneApp::MidiMapping mapping;
        mapping.channel = channelEditor->getText().getIntValue();
        mapping.type = typeComboBox->getSelectedId() == 1 ? 
                      StandaloneApp::MidiMapping::Type::ControlChange : 
                      StandaloneApp::MidiMapping::Type::Note;
        
        if (mapping.type == StandaloneApp::MidiMapping::Type::ControlChange)
        {
            mapping.ccNumber = ccEditor->getText().getIntValue();
            mapping.noteNumber = 0;  // Not used for CC
        }
        else
        {
            mapping.ccNumber = 0;  // Not used for Note
            mapping.noteNumber = noteComboBox->getSelectedId() - 1; // Convert back to 0-based MIDI note number
        }
        
        mapping.minValue = minEditor->getText().getFloatValue();
        mapping.maxValue = maxEditor->getText().getFloatValue();
        mapping.isButton = (controlItem->getControlType() == "Button");
        
        // Get current mappings and add the new one
        auto currentMappings = controlItem->getMappings();
        currentMappings.push_back(mapping);
        controlItem->updateMappings(currentMappings);
        
        // Update app mappings
        updateAppMappings();
        
        if (auto* dialogWindow = content->findParentComponentOfClass<juce::DialogWindow>())
            dialogWindow->closeButtonPressed();
    };
    
    cancelButton->onClick = [content]()
    {
        if (auto* dialogWindow = content->findParentComponentOfClass<juce::DialogWindow>())
            dialogWindow->closeButtonPressed();
    };
    
    options.content.setOwned(content);
    options.content->setSize(300, 250);
    options.dialogTitle = "Add MIDI Mapping";
    options.dialogBackgroundColour = juce::Colours::white;
    options.escapeKeyTriggersCloseButton = true;
    options.useNativeTitleBar = true;
    options.resizable = false;
    
    auto* window = options.launchAsync();
    window->centreWithSize(300, 250);
}

void MidiMappingAccordion::removeMapping(ControlItem* controlItem)
{
    auto currentMappings = controlItem->getMappings();
    if (!currentMappings.empty())
    {
        currentMappings.pop_back();
        controlItem->updateMappings(currentMappings);
        updateAppMappings();
    }
}

juce::String MidiMappingAccordion::getControlName(const juce::String& controlType, int index) const
{
    if (controlType == "Axis")
    {
        switch (index)
        {
            case 0: return "Left Stick X";
            case 1: return "Left Stick Y";
            case 2: return "Right Stick X";
            case 3: return "Right Stick Y";
            case 4: return "L2 Trigger";
            case 5: return "R2 Trigger";
            default: return "Axis " + juce::String(index);
        }
    }
    else if (controlType == "Button")
    {
        switch (index)
        {
            case 0: return "A Button";
            case 1: return "B Button";
            case 2: return "X Button";
            case 3: return "Y Button";
            case 9: return "L1 Button";
            case 10: return "R1 Button";
            case 11: return "D-pad Up";
            case 12: return "D-pad Down";
            case 13: return "D-pad Left";
            case 14: return "D-pad Right";
            case 7: return "Left Stick Button";
            case 8: return "Right Stick Button";
            default: return "Button " + juce::String(index);
        }
    }
    else if (controlType == "Gyro")
    {
        switch (index)
        {
            case 0: return "Gyroscope X";
            case 1: return "Gyroscope Y";
            case 2: return "Gyroscope Z";
            default: return "Gyro " + juce::String(index);
        }
    }
    else if (controlType == "Accel")
    {
        switch (index)
        {
            case 0: return "Accelerometer X";
            case 1: return "Accelerometer Y";
            case 2: return "Accelerometer Z";
            default: return "Accel " + juce::String(index);
        }
    }
    
    return controlType + " " + juce::String(index);
}

void MidiMappingAccordion::updateAppMappings()
{
    // Clear existing mappings first
    for (auto& mappings : app.axisMappings) mappings.clear();
    for (auto& mappings : app.buttonMappings) mappings.clear();
    for (auto& mappings : app.gyroMappings) mappings.clear();
    for (auto& mappings : app.accelerometerMappings) mappings.clear();
    
    // Update mappings
    for (const auto& item : controlItems)
    {
        const auto& mappings = item->getMappings();
        const auto& controlType = item->getControlType();
        const auto& controlIndex = item->getControlIndex();
        
        if (controlType == "Axis")
        {
            if (controlIndex >= 0 && controlIndex < GamepadManager::MAX_AXES)
            {
                app.axisMappings[static_cast<size_t>(controlIndex)] = mappings;
            }
        }
        else if (controlType == "Button")
        {
            if (controlIndex >= 0 && controlIndex < GamepadManager::MAX_BUTTONS)
            {
                app.buttonMappings[static_cast<size_t>(controlIndex)] = mappings;
            }
        }
        else if (controlType == "Gyro")
        {
            if (controlIndex >= 0 && controlIndex < 3)
            {
                app.gyroMappings[static_cast<size_t>(controlIndex)] = mappings;
            }
        }
        else if (controlType == "Accel")
        {
            if (controlIndex >= 0 && controlIndex < 3)
            {
                app.accelerometerMappings[static_cast<size_t>(controlIndex)] = mappings;
            }
        }
    }
    
    // Notify the gamepad component that mappings have changed
    app.updateMidiMappings();
}

void MidiMappingAccordion::saveMappings()
{
    juce::FileChooser chooser("Save MIDI Mappings",
                             juce::File::getSpecialLocation(juce::File::userHomeDirectory),
                             "*.json");
                             
    if (chooser.browseForFileToSave(true))
    {
        juce::File file = chooser.getResult();
        
        juce::DynamicObject::Ptr jsonObj = new juce::DynamicObject();
        
        // Convert mapping data to JSON
        juce::Array<juce::var> mappingsArray;
        for (const auto& item : controlItems)
        {
            juce::DynamicObject::Ptr mappingObj = new juce::DynamicObject();
            mappingObj->setProperty("controlName", item->getName());
            mappingObj->setProperty("controlType", item->getControlType());
            mappingObj->setProperty("controlIndex", item->getControlIndex());
            
            juce::Array<juce::var> midiMappingsArray;
            for (const auto& mapping : item->getMappings())
            {
                juce::DynamicObject::Ptr midiMappingObj = new juce::DynamicObject();
                midiMappingObj->setProperty("type", static_cast<int>(mapping.type));
                midiMappingObj->setProperty("channel", mapping.channel);
                midiMappingObj->setProperty("ccNumber", mapping.ccNumber);
                midiMappingObj->setProperty("noteNumber", mapping.noteNumber);
                midiMappingObj->setProperty("minValue", mapping.minValue);
                midiMappingObj->setProperty("maxValue", mapping.maxValue);
                midiMappingObj->setProperty("isButton", mapping.isButton);
                midiMappingsArray.add(juce::var(midiMappingObj));
            }
            mappingObj->setProperty("mappings", midiMappingsArray);
            mappingsArray.add(juce::var(mappingObj));
        }
        
        jsonObj->setProperty("mappings", mappingsArray);
        
        // Convert to JSON string with proper formatting
        juce::String jsonString = juce::JSON::toString(juce::var(jsonObj), true);
        
        // Write to file
        file.replaceWithText(jsonString);
    }
}

void MidiMappingAccordion::loadMappings()
{
    juce::FileChooser chooser("Load MIDI Mappings",
                             juce::File::getSpecialLocation(juce::File::userHomeDirectory),
                             "*.json");
                             
    if (chooser.browseForFileToOpen())
    {
        juce::File file = chooser.getResult();
        
        juce::var json = juce::JSON::parse(file);
        if (json.isVoid())
        {
            return;
        }
        
        if (auto* obj = json.getDynamicObject())
        {
            if (auto* mappingsVar = obj->getProperty("mappings").getArray())
            {
                // Clear existing mappings
                for (auto& item : controlItems)
                {
                    item->updateMappings(std::vector<StandaloneApp::MidiMapping>());
                }
                
                // Load mappings
                for (const auto& mappingVar : *mappingsVar)
                {
                    if (auto* mappingObj = mappingVar.getDynamicObject())
                    {
                        juce::String controlType = mappingObj->getProperty("controlType").toString();
                        int controlIndex = mappingObj->getProperty("controlIndex");
                        
                        // Find the matching control item
                        for (auto& item : controlItems)
                        {
                            if (item->getControlType() == controlType && item->getControlIndex() == controlIndex)
                            {
                                if (auto* midiMappingsVar = mappingObj->getProperty("mappings").getArray())
                                {
                                    std::vector<StandaloneApp::MidiMapping> mappings;
                                    
                                    for (const auto& midiMappingVar : *midiMappingsVar)
                                    {
                                        if (auto* midiMappingObj = midiMappingVar.getDynamicObject())
                                        {
                                            StandaloneApp::MidiMapping mapping;
                                            
                                            if (midiMappingObj->hasProperty("type"))
                                            {
                                                mapping.type = static_cast<StandaloneApp::MidiMapping::Type>(
                                                    midiMappingObj->getProperty("type").operator int());
                                            }
                                            else
                                            {
                                                mapping.type = StandaloneApp::MidiMapping::Type::ControlChange;
                                            }
                                            
                                            mapping.channel = midiMappingObj->getProperty("channel");
                                            mapping.ccNumber = midiMappingObj->getProperty("ccNumber");
                                            mapping.noteNumber = midiMappingObj->hasProperty("noteNumber") ? 
                                                static_cast<int>(midiMappingObj->getProperty("noteNumber")) : 0;
                                            mapping.minValue = midiMappingObj->getProperty("minValue");
                                            mapping.maxValue = midiMappingObj->getProperty("maxValue");
                                            mapping.isButton = midiMappingObj->getProperty("isButton");
                                            
                                            mappings.push_back(mapping);
                                        }
                                    }
                                    
                                    item->updateMappings(mappings);
                                }
                                
                                break;
                            }
                        }
                    }
                }
                
                updateAppMappings();
            }
        }
    }
}

void MidiMappingAccordion::highlightControl(const juce::String& controlType, int controlIndex)
{
    // Find the control item that matches the control type and index
    for (auto& item : controlItems)
    {
        if (item->getControlType() == controlType && item->getControlIndex() == controlIndex)
        {
            // Highlight the control and make it visible
            item->setHighlighted(true);
            item->setExpanded(true);
            
            // Scroll to make the item visible
            viewport.setViewPosition(0, item->getY());
            
            // Start a timer to remove the highlight after a short time
            juce::Timer::callAfterDelay(1000, [this, item = item.get()]() {
                item->setHighlighted(false);
            });
            
            break;
        }
    }
}

juce::String MidiMappingAccordion::getMidiNoteName(int midiNoteNumber)
{
    if (midiNoteNumber < 0 || midiNoteNumber > 127)
        return "Invalid";
        
    const char* noteNames[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    int octave = (midiNoteNumber / 12) - 1;
    int noteIndex = midiNoteNumber % 12;
    
    return juce::String(noteNames[noteIndex]) + juce::String(octave);
} 