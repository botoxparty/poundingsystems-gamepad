#include "MidiMappingEditor.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_core/juce_core.h>

// Column IDs
enum ColumnIDs
{
    ControlNameColumn = 1,
    ControlTypeColumn,
    MappingColumn
};

MidiMappingEditor::MidiMappingEditor(StandaloneApp& app)
    : app(app)
{
    // Set up the table
    mappingTable.setModel(this);
    mappingTable.setColour(juce::ListBox::backgroundColourId, juce::Colours::white);
    mappingTable.setColour(juce::ListBox::outlineColourId, juce::Colours::grey);
    mappingTable.setColour(juce::TableListBox::backgroundColourId, juce::Colours::lightgrey);
    mappingTable.getHeader().addColumn("Control", ControlNameColumn, 150, 150, 150);
    mappingTable.getHeader().addColumn("Type", ControlTypeColumn, 100, 100, 100);
    mappingTable.getHeader().addColumn("MIDI Mappings", MappingColumn, 300, 300, 300);
    addAndMakeVisible(mappingTable);
    
    // Set up buttons
    addMappingButton.setButtonText("Add Mapping");
    addMappingButton.addListener(this);
    addAndMakeVisible(addMappingButton);
    
    removeMappingButton.setButtonText("Remove Mapping");
    removeMappingButton.addListener(this);
    addAndMakeVisible(removeMappingButton);
    
    saveButton.setButtonText("Save");
    saveButton.addListener(this);
    addAndMakeVisible(saveButton);
    
    loadButton.setButtonText("Load");
    loadButton.addListener(this);
    addAndMakeVisible(loadButton);
    
    // Initialize mapping data
    updateMappingData();
}

MidiMappingEditor::~MidiMappingEditor()
{
    mappingTable.setModel(nullptr);
}

void MidiMappingEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::white);
}

void MidiMappingEditor::resized()
{
    auto area = getLocalBounds();
    
    // Position buttons at the bottom
    auto buttonArea = area.removeFromBottom(40);
    buttonArea.reduce(10, 10);
    
    addMappingButton.setBounds(buttonArea.removeFromLeft(100));
    buttonArea.removeFromLeft(10);
    removeMappingButton.setBounds(buttonArea.removeFromLeft(100));
    buttonArea.removeFromLeft(10);
    saveButton.setBounds(buttonArea.removeFromLeft(100));
    buttonArea.removeFromLeft(10);
    loadButton.setBounds(buttonArea.removeFromLeft(100));
    
    // Position table
    area.removeFromBottom(10);
    mappingTable.setBounds(area);
}

int MidiMappingEditor::getNumRows()
{
    return mappingData.size();
}

void MidiMappingEditor::paintRowBackground(juce::Graphics& g, int rowNumber, int width, int height, bool rowIsSelected)
{
    if (rowIsSelected)
        g.fillAll(juce::Colours::lightblue);
    else
        g.fillAll(juce::Colours::white);
}

void MidiMappingEditor::paintCell(juce::Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected)
{
    g.setColour(rowIsSelected ? juce::Colours::black : juce::Colours::darkgrey);
    
    const auto& data = mappingData[rowNumber];
    
    switch (columnId)
    {
        case ControlNameColumn:
            g.drawText(data.controlName, 2, 0, width - 4, height, juce::Justification::centredLeft, true);
            break;
            
        case ControlTypeColumn:
            g.drawText(data.controlType, 2, 0, width - 4, height, juce::Justification::centredLeft, true);
            break;
            
        case MappingColumn:
        {
            juce::String mappingText;
            for (size_t i = 0; i < data.mappings.size(); ++i)
            {
                const auto& mapping = data.mappings[i];
                mappingText += juce::String("Ch:") + juce::String(mapping.channel) +
                             " CC:" + juce::String(mapping.ccNumber) +
                             " [" + juce::String(mapping.minValue) + "-" + juce::String(mapping.maxValue) + "]";
                             
                if (i < data.mappings.size() - 1)
                    mappingText += ", ";
            }
            g.drawText(mappingText, 2, 0, width - 4, height, juce::Justification::centredLeft, true);
            break;
        }
    }
}

juce::Component* MidiMappingEditor::refreshComponentForCell(int rowNumber, int columnId, bool isRowSelected, juce::Component* existingComponentToUpdate)
{
    return nullptr; // We don't need custom components for this table
}

void MidiMappingEditor::buttonClicked(juce::Button* button)
{
    if (button == &addMappingButton)
    {
        auto selectedRow = mappingTable.getSelectedRow();
        if (selectedRow >= 0)
            addMapping(selectedRow);
    }
    else if (button == &removeMappingButton)
    {
        auto selectedRow = mappingTable.getSelectedRow();
        if (selectedRow >= 0)
            removeMapping(selectedRow);
    }
    else if (button == &saveButton)
    {
        saveMappings();
    }
    else if (button == &loadButton)
    {
        loadMappings();
    }
}

void MidiMappingEditor::textEditorTextChanged(juce::TextEditor& editor)
{
    // Not used in this implementation
}

void MidiMappingEditor::textEditorReturnKeyPressed(juce::TextEditor& editor)
{
    // Not used in this implementation
}

void MidiMappingEditor::textEditorEscapeKeyPressed(juce::TextEditor& editor)
{
    // Not used in this implementation
}

void MidiMappingEditor::textEditorFocusLost(juce::TextEditor& editor)
{
    // Not used in this implementation
}

void MidiMappingEditor::updateMappingData()
{
    mappingData.clear();
    
    // Add axis mappings
    for (int i = 0; i < GamepadManager::MAX_AXES; ++i)
    {
        MappingData data;
        data.controlName = getControlName("Axis", i);
        data.controlType = "Axis";
        data.controlIndex = i;
        data.mappings = app.axisMappings[static_cast<size_t>(i)];
        mappingData.push_back(data);
    }
    
    // Add button mappings
    for (int i = 0; i < GamepadManager::MAX_BUTTONS; ++i)
    {
        MappingData data;
        data.controlName = getControlName("Button", i);
        data.controlType = "Button";
        data.controlIndex = i;
        data.mappings = app.buttonMappings[static_cast<size_t>(i)];
        mappingData.push_back(data);
    }

    // Add gyroscope mappings
    for (int i = 0; i < 3; ++i)
    {
        MappingData data;
        data.controlName = getControlName("Gyro", i);
        data.controlType = "Gyro";
        data.controlIndex = i;
        data.mappings = app.gyroMappings[static_cast<size_t>(i)];
        mappingData.push_back(data);
    }

    // Add accelerometer mappings
    for (int i = 0; i < 3; ++i)
    {
        MappingData data;
        data.controlName = getControlName("Accel", i);
        data.controlType = "Accel";
        data.controlIndex = i;
        data.mappings = app.accelerometerMappings[static_cast<size_t>(i)];
        mappingData.push_back(data);
    }
    
    mappingTable.updateContent();
}

void MidiMappingEditor::addMapping(int rowIndex)
{
    if (rowIndex < 0 || static_cast<size_t>(rowIndex) >= mappingData.size())
        return;
        
    auto& data = mappingData[rowIndex];
    
    // Create a dialog to get mapping details
    juce::DialogWindow::LaunchOptions options;
    auto* content = new juce::Component();
    content->setSize(300, 250);
    
    auto* channelLabel = new juce::Label("channel", "MIDI Channel:");
    auto* channelEditor = new juce::TextEditor();
    channelEditor->setText("1");
    
    auto* ccLabel = new juce::Label("cc", "CC Number:");
    auto* ccEditor = new juce::TextEditor();
    ccEditor->setText("1");
    
    auto* minLabel = new juce::Label("min", "Min Value:");
    auto* minEditor = new juce::TextEditor();
    minEditor->setText("0");
    
    auto* maxLabel = new juce::Label("max", "Max Value:");
    auto* maxEditor = new juce::TextEditor();
    maxEditor->setText("127");
    
    auto* okButton = new juce::TextButton("OK");
    auto* cancelButton = new juce::TextButton("Cancel");
    
    content->addAndMakeVisible(channelLabel);
    content->addAndMakeVisible(channelEditor);
    content->addAndMakeVisible(ccLabel);
    content->addAndMakeVisible(ccEditor);
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
    ccLabel->setBounds(bounds.removeFromTop(20));
    ccEditor->setBounds(bounds.removeFromTop(20));
    bounds.removeFromTop(10);
    minLabel->setBounds(bounds.removeFromTop(20));
    minEditor->setBounds(bounds.removeFromTop(20));
    bounds.removeFromTop(10);
    maxLabel->setBounds(bounds.removeFromTop(20));
    maxEditor->setBounds(bounds.removeFromTop(20));
    bounds.removeFromTop(10);
    
    auto buttonArea = bounds.removeFromBottom(30);
    okButton->setBounds(buttonArea.removeFromLeft(100));
    buttonArea.removeFromLeft(10);
    cancelButton->setBounds(buttonArea.removeFromLeft(100));
    
    // Handle button clicks
    okButton->onClick = [this, content, channelEditor, ccEditor, minEditor, maxEditor, &data]()
    {
        StandaloneApp::MidiMapping mapping;
        mapping.channel = channelEditor->getText().getIntValue();
        mapping.ccNumber = ccEditor->getText().getIntValue();
        mapping.minValue = minEditor->getText().getFloatValue();
        mapping.maxValue = maxEditor->getText().getFloatValue();
        mapping.isButton = (data.controlType == "Button");
        
        data.mappings.push_back(mapping);
        updateAppMappings();
        mappingTable.updateContent();
        
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
    options.dialogBackgroundColour = juce::Colours::lightgrey;
    options.escapeKeyTriggersCloseButton = true;
    options.useNativeTitleBar = true;
    options.resizable = false;
    
    auto* window = options.launchAsync();
    window->centreWithSize(300, 250);
}

void MidiMappingEditor::removeMapping(int rowIndex)
{
    if (rowIndex < 0 || static_cast<size_t>(rowIndex) >= mappingData.size())
        return;
        
    auto& data = mappingData[rowIndex];
    if (!data.mappings.empty())
    {
        data.mappings.pop_back();
        updateAppMappings();
        mappingTable.updateContent();
    }
}

juce::String MidiMappingEditor::getControlName(const juce::String& controlType, int index) const
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

void MidiMappingEditor::updateAppMappings()
{
    // Update axis mappings
    for (const auto& data : mappingData)
    {
        if (data.controlType == "Axis")
            app.axisMappings[static_cast<size_t>(data.controlIndex)] = data.mappings;
        else if (data.controlType == "Button")
            app.buttonMappings[static_cast<size_t>(data.controlIndex)] = data.mappings;
        else if (data.controlType == "Gyro")
            app.gyroMappings[static_cast<size_t>(data.controlIndex)] = data.mappings;
        else if (data.controlType == "Accel")
            app.accelerometerMappings[static_cast<size_t>(data.controlIndex)] = data.mappings;
    }
}

void MidiMappingEditor::saveMappings()
{
    juce::FileChooser chooser("Save MIDI Mappings",
                             juce::File::getSpecialLocation(juce::File::userHomeDirectory),
                             "*.json");
                             
    if (chooser.browseForFileToSave(true))
    {
        juce::File file = chooser.getResult();
        juce::DynamicObject::Ptr jsonObj = new juce::DynamicObject();
        juce::var json(jsonObj);
        
        // Convert mapping data to JSON
        juce::Array<juce::var> mappingsArray;
        for (const auto& data : mappingData)
        {
            juce::DynamicObject::Ptr mappingObj = new juce::DynamicObject();
            mappingObj->setProperty("controlName", data.controlName);
            mappingObj->setProperty("controlType", data.controlType);
            mappingObj->setProperty("controlIndex", data.controlIndex);
            
            juce::Array<juce::var> midiMappingsArray;
            for (const auto& mapping : data.mappings)
            {
                juce::DynamicObject::Ptr midiMappingObj = new juce::DynamicObject();
                midiMappingObj->setProperty("channel", mapping.channel);
                midiMappingObj->setProperty("ccNumber", mapping.ccNumber);
                midiMappingObj->setProperty("minValue", mapping.minValue);
                midiMappingObj->setProperty("maxValue", mapping.maxValue);
                midiMappingObj->setProperty("isButton", mapping.isButton);
                midiMappingsArray.add(juce::var(midiMappingObj));
            }
            mappingObj->setProperty("mappings", midiMappingsArray);
            mappingsArray.add(juce::var(mappingObj));
        }
        
        jsonObj->setProperty("mappings", mappingsArray);
        
        // Write to file
        file.replaceWithText(json.toString());
    }
}

void MidiMappingEditor::loadMappings()
{
    juce::FileChooser chooser("Load MIDI Mappings",
                             juce::File::getSpecialLocation(juce::File::userHomeDirectory),
                             "*.json");
                             
    if (chooser.browseForFileToOpen())
    {
        juce::File file = chooser.getResult();
        juce::var json = juce::JSON::parse(file);
        
        if (auto* obj = json.getDynamicObject())
        {
            if (auto* mappingsVar = obj->getProperty("mappings").getArray())
            {
                mappingData.clear();
                
                for (const auto& mappingVar : *mappingsVar)
                {
                    if (auto* mappingObj = mappingVar.getDynamicObject())
                    {
                        MappingData data;
                        data.controlName = mappingObj->getProperty("controlName").toString();
                        data.controlType = mappingObj->getProperty("controlType").toString();
                        data.controlIndex = mappingObj->getProperty("controlIndex");
                        
                        if (auto* midiMappingsVar = mappingObj->getProperty("mappings").getArray())
                        {
                            for (const auto& midiMappingVar : *midiMappingsVar)
                            {
                                if (auto* midiMappingObj = midiMappingVar.getDynamicObject())
                                {
                                    StandaloneApp::MidiMapping mapping;
                                    mapping.channel = midiMappingObj->getProperty("channel");
                                    mapping.ccNumber = midiMappingObj->getProperty("ccNumber");
                                    mapping.minValue = midiMappingObj->getProperty("minValue");
                                    mapping.maxValue = midiMappingObj->getProperty("maxValue");
                                    mapping.isButton = midiMappingObj->getProperty("isButton");
                                    data.mappings.push_back(mapping);
                                }
                            }
                        }
                        
                        mappingData.push_back(data);
                    }
                }
                
                updateAppMappings();
                mappingTable.updateContent();
            }
        }
    }
} 