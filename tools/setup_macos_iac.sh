#!/bin/bash
# Script to set up the IAC Driver on macOS

echo "==== Setting up IAC Driver for MIDI ====="
echo "This script will help you set up the IAC Driver on macOS"
echo "The IAC Driver provides virtual MIDI ports that can be used instead of creating custom virtual devices"
echo ""

echo "Step 1: Opening Audio MIDI Setup application"
open -a "Audio MIDI Setup"

echo ""
echo "Step 2: Instructions"
echo "1. In Audio MIDI Setup, press Cmd+2 or go to Window > Show MIDI Studio"
echo "2. Double-click on the 'IAC Driver' icon"
echo "3. Check the 'Device is online' checkbox"
echo "4. Click the '+' button to add at least one port"
echo "5. Optionally rename the port to 'Gamepad MIDI Controller'"
echo "6. Click 'Apply' and close the window"
echo ""
echo "After setting up the IAC Driver, restart the Gamepad MIDI Controller application"
echo "It should automatically detect and use the IAC Driver port"
echo ""
echo "Press Enter after you've completed these steps..."
read 