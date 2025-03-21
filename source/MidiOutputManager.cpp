#include "MidiOutputManager.h"

MidiOutputManager::MidiOutputManager()
{
}

MidiOutputManager::~MidiOutputManager()
{
    midiOutput.reset();
}

juce::Array<juce::MidiDeviceInfo> MidiOutputManager::getAvailableDevices() const
{
    return juce::MidiOutput::getAvailableDevices();
}

bool MidiOutputManager::setOutputDevice(const juce::String& deviceIdentifier)
{
    midiOutput.reset();
    
    if (deviceIdentifier.isEmpty())
        return false;
    
    midiOutput = juce::MidiOutput::openDevice(deviceIdentifier);
    
    if (midiOutput != nullptr)
    {
        currentDeviceInfo = midiOutput->getDeviceInfo();
        midiOutput->startBackgroundThread();
        return true;
    }
    
    return false;
}

void MidiOutputManager::sendControlChange(int channel, int controller, int value)
{
    if (midiOutput != nullptr)
    {
        juce::MidiMessage message = juce::MidiMessage::controllerEvent(channel, controller, value);
        midiOutput->sendMessageNow(message);
    }
}

void MidiOutputManager::sendNoteOn(int channel, int noteNumber, float velocity)
{
    if (midiOutput != nullptr)
    {
        auto message = juce::MidiMessage::noteOn(channel, noteNumber, velocity);
        midiOutput->sendMessageNow(message);
    }
}

void MidiOutputManager::sendNoteOff(int channel, int noteNumber)
{
    if (midiOutput != nullptr)
    {
        juce::MidiMessage message = juce::MidiMessage::noteOff(channel, noteNumber);
        midiOutput->sendMessageNow(message);
    }
} 