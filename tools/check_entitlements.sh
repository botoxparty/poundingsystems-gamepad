#!/bin/bash
# Script to check entitlements of a built application

# Set the app path - adjust this to your build location
APP_PATH="./build/GamepadMIDI_artefacts/Release/Gamepad MIDI.app"

echo "==== Checking code signing for $APP_PATH ===="
codesign -dvv "$APP_PATH" 2>&1

echo ""
echo "==== Checking entitlements ===="
codesign -d --entitlements :- "$APP_PATH" 2>&1

echo ""
echo "==== Checking Info.plist ===="
plutil -p "$APP_PATH/Contents/Info.plist"

echo ""
echo "==== Current available MIDI devices ===="
system_profiler SPMIDIDataType

echo ""
echo "==== CoreAudio permissions ===="
# Check if the app is explicitly listed in TCC.db
echo "Note: This requires admin access to properly check"
sudo sqlite3 /Library/Application\ Support/com.apple.TCC/TCC.db \
  "SELECT * FROM access WHERE client LIKE '%${BUNDLE_ID}%';" 2>/dev/null || \
  echo "Unable to check TCC database (requires sudo)." 