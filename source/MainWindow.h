#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "StandaloneApp.h"

class MainWindow : public juce::DocumentWindow
{
public:
    MainWindow(const juce::String& name)
        : DocumentWindow(name,
                        juce::Colours::darkgrey,
                        DocumentWindow::allButtons)
    {
        setUsingNativeTitleBar(true);
        setContentOwned(new StandaloneApp(), true);
        
        setResizable(false, false);
        centreWithSize(600, 440);
        setVisible(true);
    }
    
    void closeButtonPressed() override
    {
        juce::JUCEApplication::getInstance()->systemRequestedQuit();
    }
    
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
}; 