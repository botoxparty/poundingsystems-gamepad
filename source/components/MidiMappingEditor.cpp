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
                
                if (mapping.type == StandaloneApp::MidiMapping::Type::ControlChange)
                {
                    mappingText += juce::String("Ch:") + juce::String(mapping.channel) +
                                 " CC:" + juce::String(mapping.ccNumber) +
                                 " [" + juce::String(mapping.minValue) + "-" + juce::String(mapping.maxValue) + "]";
                }
                else // Note
                {
                    mappingText += juce::String("Ch:") + juce::String(mapping.channel) +
                                 " Note:" + juce::String(mapping.noteNumber) +
                                 " [" + juce::String(mapping.minValue) + "-" + juce::String(mapping.maxValue) + "]";
                }
                                 
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
    content->setSize(300, 300);
    
    auto* channelLabel = new juce::Label("channel", "MIDI Channel:");
    auto* channelEditor = new juce::TextEditor();
    channelEditor->setText("1");
    
    auto* typeLabel = new juce::Label("type", "MIDI Type:");
    auto* typeComboBox = new juce::ComboBox("typeComboBox");
    typeComboBox->addItem("Control Change (CC)", 1);
    typeComboBox->addItem("Note", 2);
    typeComboBox->setSelectedId(1);
    
    auto* ccLabel = new juce::Label("cc", "CC Number:");
    auto* ccEditor = new juce::TextEditor();
    ccEditor->setText("1");
    
    auto* noteLabel = new juce::Label("note", "Note Number:");
    auto* noteEditor = new juce::TextEditor();
    noteEditor->setText("60");  // Middle C
    noteEditor->setEnabled(false);  // Initially disabled
    
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
    content->addAndMakeVisible(typeLabel);
    content->addAndMakeVisible(typeComboBox);
    content->addAndMakeVisible(ccLabel);
    content->addAndMakeVisible(ccEditor);
    content->addAndMakeVisible(noteLabel);
    content->addAndMakeVisible(noteEditor);
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
    
    ccLabel->setBounds(bounds.removeFromTop(20));
    ccEditor->setBounds(bounds.removeFromTop(20));
    bounds.removeFromTop(10);
    
    noteLabel->setBounds(bounds.removeFromTop(20));
    noteEditor->setBounds(bounds.removeFromTop(20));
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
    
    // Handle type selection change
    typeComboBox->onChange = [ccEditor, noteEditor, typeComboBox]() {
        bool isCC = typeComboBox->getSelectedId() == 1;
        ccEditor->setEnabled(isCC);
        noteEditor->setEnabled(!isCC);
    };
    
    // Handle button clicks
    okButton->onClick = [this, content, channelEditor, typeComboBox, ccEditor, noteEditor, minEditor, maxEditor, &data]()
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
            mapping.noteNumber = noteEditor->getText().getIntValue();
        }
        
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
    options.content->setSize(300, 300);
    options.dialogTitle = "Add MIDI Mapping";
    options.dialogBackgroundColour = juce::Colours::lightgrey;
    options.escapeKeyTriggersCloseButton = true;
    options.useNativeTitleBar = true;
    options.resizable = false;
    
    auto* window = options.launchAsync();
    window->centreWithSize(300, 300);
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
    // Clear existing mappings first
    for (auto& mappings : app.axisMappings) mappings.clear();
    for (auto& mappings : app.buttonMappings) mappings.clear();
    for (auto& mappings : app.gyroMappings) mappings.clear();
    for (auto& mappings : app.accelerometerMappings) mappings.clear();
    
    // Update mappings
    for (const auto& data : mappingData)
    {
        juce::Logger::writeToLog("Updating mapping for " + data.controlName + 
                                " (Type: " + data.controlType + 
                                ", Index: " + juce::String(data.controlIndex) + ")");
        
        if (data.controlType == "Axis")
        {
            if (data.controlIndex >= 0 && data.controlIndex < GamepadManager::MAX_AXES)
            {
                app.axisMappings[static_cast<size_t>(data.controlIndex)] = data.mappings;
                juce::Logger::writeToLog("Added " + juce::String(data.mappings.size()) + " axis mappings");
            }
            else
            {
                juce::Logger::writeToLog("WARNING: Invalid axis index: " + juce::String(data.controlIndex));
            }
        }
        else if (data.controlType == "Button")
        {
            if (data.controlIndex >= 0 && data.controlIndex < GamepadManager::MAX_BUTTONS)
            {
                app.buttonMappings[static_cast<size_t>(data.controlIndex)] = data.mappings;
                juce::Logger::writeToLog("Added " + juce::String(data.mappings.size()) + " button mappings");
            }
            else
            {
                juce::Logger::writeToLog("WARNING: Invalid button index: " + juce::String(data.controlIndex));
            }
        }
        else if (data.controlType == "Gyro")
        {
            if (data.controlIndex >= 0 && data.controlIndex < 3)
            {
                app.gyroMappings[static_cast<size_t>(data.controlIndex)] = data.mappings;
                juce::Logger::writeToLog("Added " + juce::String(data.mappings.size()) + " gyro mappings");
            }
            else
            {
                juce::Logger::writeToLog("WARNING: Invalid gyro index: " + juce::String(data.controlIndex));
            }
        }
        else if (data.controlType == "Accel")
        {
            if (data.controlIndex >= 0 && data.controlIndex < 3)
            {
                app.accelerometerMappings[static_cast<size_t>(data.controlIndex)] = data.mappings;
                juce::Logger::writeToLog("Added " + juce::String(data.mappings.size()) + " accelerometer mappings");
            }
            else
            {
                juce::Logger::writeToLog("WARNING: Invalid accelerometer index: " + juce::String(data.controlIndex));
            }
        }
    }
    
    // Notify the gamepad component that mappings have changed
    app.updateMidiMappings();
    juce::Logger::writeToLog("MIDI mappings updated and gamepad component notified");
}

void MidiMappingEditor::saveMappings()
{
    juce::Logger::writeToLog("Starting to save MIDI mappings...");
    
    juce::FileChooser chooser("Save MIDI Mappings",
                             juce::File::getSpecialLocation(juce::File::userHomeDirectory),
                             "*.json");
                             
    if (chooser.browseForFileToSave(true))
    {
        juce::File file = chooser.getResult();
        juce::Logger::writeToLog("Saving to file: " + file.getFullPathName());
        
        juce::DynamicObject::Ptr jsonObj = new juce::DynamicObject();
        
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
        juce::Logger::writeToLog("Generated JSON: " + jsonString);
        
        // Write to file
        if (file.replaceWithText(jsonString))
        {
            juce::Logger::writeToLog("Successfully saved MIDI mappings to file");
        }
        else
        {
            juce::Logger::writeToLog("ERROR: Failed to write to file");
        }
    }
    else
    {
        juce::Logger::writeToLog("Save operation cancelled");
    }
}

void MidiMappingEditor::loadMappings()
{
    juce::Logger::writeToLog("Starting to load MIDI mappings...");
    
    juce::FileChooser chooser("Load MIDI Mappings",
                             juce::File::getSpecialLocation(juce::File::userHomeDirectory),
                             "*.json");
                             
    if (chooser.browseForFileToOpen())
    {
        juce::File file = chooser.getResult();
        juce::Logger::writeToLog("Selected file: " + file.getFullPathName());
        
        juce::var json = juce::JSON::parse(file);
        if (json.isVoid())
        {
            juce::Logger::writeToLog("ERROR: Failed to parse JSON file");
            return;
        }
        
        juce::Logger::writeToLog("Successfully parsed JSON file");
        
        if (auto* obj = json.getDynamicObject())
        {
            if (auto* mappingsVar = obj->getProperty("mappings").getArray())
            {
                juce::Logger::writeToLog("Found " + juce::String(mappingsVar->size()) + " mappings to load");
                mappingData.clear();
                
                for (const auto& mappingVar : *mappingsVar)
                {
                    if (auto* mappingObj = mappingVar.getDynamicObject())
                    {
                        MappingData data;
                        data.controlName = mappingObj->getProperty("controlName").toString();
                        data.controlType = mappingObj->getProperty("controlType").toString();
                        data.controlIndex = mappingObj->getProperty("controlIndex");
                        
                        juce::Logger::writeToLog("Loading mapping for " + data.controlName + 
                                               " (Type: " + data.controlType + 
                                               ", Index: " + juce::String(data.controlIndex) + ")");
                        
                        if (auto* midiMappingsVar = mappingObj->getProperty("mappings").getArray())
                        {
                            juce::Logger::writeToLog("Found " + juce::String(midiMappingsVar->size()) + 
                                                   " MIDI mappings for " + data.controlName);
                            
                            for (const auto& midiMappingVar : *midiMappingsVar)
                            {
                                if (auto* midiMappingObj = midiMappingVar.getDynamicObject())
                                {
                                    StandaloneApp::MidiMapping mapping;
                                    
                                    // Handle the type field, with backward compatibility
                                    if (midiMappingObj->hasProperty("type"))
                                    {
                                        mapping.type = static_cast<StandaloneApp::MidiMapping::Type>(
                                            midiMappingObj->getProperty("type").operator int());
                                        juce::String typeStr = (mapping.type == StandaloneApp::MidiMapping::Type::ControlChange) ? 
                                            "Control Change" : "Note";
                                        juce::Logger::writeToLog("Mapping type: " + typeStr);
                                    }
                                    else
                                    {
                                        mapping.type = StandaloneApp::MidiMapping::Type::ControlChange;
                                        juce::Logger::writeToLog("Using default mapping type: Control Change");
                                    }
                                    
                                    mapping.channel = midiMappingObj->getProperty("channel");
                                    mapping.ccNumber = midiMappingObj->getProperty("ccNumber");
                                    mapping.noteNumber = midiMappingObj->hasProperty("noteNumber") ? 
                                        static_cast<int>(midiMappingObj->getProperty("noteNumber")) : 0;
                                    mapping.minValue = midiMappingObj->getProperty("minValue");
                                    mapping.maxValue = midiMappingObj->getProperty("maxValue");
                                    mapping.isButton = midiMappingObj->getProperty("isButton");
                                    
                                    juce::Logger::writeToLog("Added MIDI mapping - Channel: " + juce::String(mapping.channel) + 
                                                           ", CC: " + juce::String(mapping.ccNumber) + 
                                                           ", Note: " + juce::String(mapping.noteNumber) + 
                                                           ", Range: [" + juce::String(mapping.minValue) + 
                                                           "-" + juce::String(mapping.maxValue) + "]");
                                    
                                    data.mappings.push_back(mapping);
                                }
                            }
                        }
                        else
                        {
                            juce::Logger::writeToLog("WARNING: No MIDI mappings found for " + data.controlName);
                        }
                        
                        mappingData.push_back(data);
                    }
                }
                
                juce::Logger::writeToLog("Finished loading all mappings, updating app...");
                updateAppMappings();
                mappingTable.updateContent();
                juce::Logger::writeToLog("MIDI mappings loaded and UI updated");
            }
            else
            {
                juce::Logger::writeToLog("ERROR: No mappings array found in JSON file");
            }
        }
        else
        {
            juce::Logger::writeToLog("ERROR: Invalid JSON structure");
        }
    }
    else
    {
        juce::Logger::writeToLog("File selection cancelled");
    }
} 