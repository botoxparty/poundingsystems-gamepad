#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_core/juce_core.h>
#include "MainWindow.h"

// Define application name and version if not already defined
#ifndef JUCE_APPLICATION_NAME_STRING
#define JUCE_APPLICATION_NAME_STRING "Gamepad MIDI"
#endif

#ifndef JUCE_APPLICATION_VERSION_STRING
#define JUCE_APPLICATION_VERSION_STRING "1.0.0"
#endif

class GamepadMidiApplication : public juce::JUCEApplication
{
public:
    GamepadMidiApplication() = default;
    
    const juce::String getApplicationName() override { return JUCE_APPLICATION_NAME_STRING; }
    const juce::String getApplicationVersion() override { return JUCE_APPLICATION_VERSION_STRING; }
    bool moreThanOneInstanceAllowed() override { return false; }
    
    void initialise(const juce::String& commandLine) override
    {
        mainWindow = std::make_unique<MainWindow>(getApplicationName());
    }
    
    void shutdown() override
    {
        mainWindow = nullptr;
    }
    
    void systemRequestedQuit() override
    {
        quit();
    }
    
    void anotherInstanceStarted(const juce::String& commandLine) override {}
    
private:
    std::unique_ptr<MainWindow> mainWindow;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GamepadMidiApplication)
};

// This macro creates the application's main() function
START_JUCE_APPLICATION(GamepadMidiApplication) 