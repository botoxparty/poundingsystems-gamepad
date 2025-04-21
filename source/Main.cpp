#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_core/juce_core.h>
// #include <melatonin_inspector/melatonin_inspector.h>
#include "MainWindow.h"

// Define application name and version if not already defined
#ifndef JUCE_APPLICATION_NAME_STRING
#define JUCE_APPLICATION_NAME_STRING "Gamepad MIDI"
#endif

#ifndef JUCE_APPLICATION_VERSION_STRING
#define JUCE_APPLICATION_VERSION_STRING "0.0.4"
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
        // Initialize file logger
        logger.reset(juce::FileLogger::createDefaultAppLogger("GamepadMIDI", "gamepad_midi.log", 
            getApplicationName() + " " + getApplicationVersion() + " - Log Started"));
        juce::Logger::setCurrentLogger(logger.get());

        mainWindow = std::make_unique<MainWindow>(getApplicationName());
        #if JUCE_DEBUG
        // inspector = std::make_unique<melatonin::Inspector>(*mainWindow);
        #endif
    }
    
    void shutdown() override
    {
        #if JUCE_DEBUG
        // inspector = nullptr;
        #endif
        mainWindow = nullptr;
        
        // Clean up logger
        juce::Logger::setCurrentLogger(nullptr);
    }
    
    void systemRequestedQuit() override
    {
        quit();
    }
    
    void anotherInstanceStarted(const juce::String& commandLine) override {}
    
private:
    std::unique_ptr<MainWindow> mainWindow;
    std::unique_ptr<juce::FileLogger> logger;
    #if JUCE_DEBUG
    // std::unique_ptr<melatonin::Inspector> inspector;
    #endif
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GamepadMidiApplication)
};

// This macro creates the application's main() function
START_JUCE_APPLICATION(GamepadMidiApplication) 