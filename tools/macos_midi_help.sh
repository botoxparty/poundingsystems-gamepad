#!/bin/bash
# Script to help diagnose MIDI issues on macOS

echo "==== macOS MIDI Device Helper ===="
echo "This script will help identify and fix issues with MIDI devices on macOS"
echo ""

echo "==== Current MIDI Devices ===="
system_profiler SPMIDIDataType

echo ""
echo "==== Checking Virtual MIDI Support ===="
echo "Critical settings for virtual MIDI devices to work:"
echo "1. App must NOT be sandboxed"
echo "2. App must have proper entitlements"
echo "3. App must be properly signed"
echo ""

echo "==== IAC Driver Status ===="
echo "The IAC Driver is a built-in virtual MIDI bus for macOS:"
if system_profiler SPMIDIDataType | grep -q "IAC Driver"; then
    echo "IAC Driver exists in your system."
    if system_profiler SPMIDIDataType | grep -A 5 "IAC Driver" | grep -q "offline"; then
        echo "WARNING: IAC Driver appears to be offline. You need to enable it."
        echo "See instructions below to enable it."
    else
        echo "IAC Driver appears to be online. Good!"
    fi
else
    echo "IAC Driver not found or not configured."
    echo "You need to enable it. See instructions below."
fi

echo ""
echo "==== How to Enable the IAC Driver ===="
echo "1. Open Audio MIDI Setup (Applications > Utilities > Audio MIDI Setup)"
echo "2. Press Cmd+2 or choose Window > Show MIDI Studio"
echo "3. Double-click the 'IAC Driver' icon"
echo "4. Check 'Device is online'"
echo "5. Click the + button to add a port if none exists"
echo "6. Close the window"
echo ""
echo "After doing this, restart the Gamepad MIDI app" 