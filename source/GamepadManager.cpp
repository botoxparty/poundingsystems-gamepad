#include "GamepadManager.h"

GamepadManager::GamepadManager()
{
    if (initSDL())
    {
        // Start polling timer (30ms interval for approximately 33fps)
        startTimer(30);
    }
}

GamepadManager::~GamepadManager()
{
    stopTimer();
    cleanupSDL();
}

bool GamepadManager::initSDL()
{
    // Initialize SDL with gamepad and joystick subsystems
    if (SDL_Init(SDL_INIT_GAMEPAD | SDL_INIT_JOYSTICK) < 0)
    {
        juce::String errorMsg = "SDL could not initialize! SDL Error: " + juce::String(SDL_GetError());
        juce::Logger::writeToLog(errorMsg);
        return false;
    }

    // Initialize sensor subsystem separately
    if (SDL_InitSubSystem(SDL_INIT_SENSOR) < 0)
    {
        juce::String errorMsg = "SDL sensor subsystem could not initialize! SDL Error: " + juce::String(SDL_GetError());
        juce::Logger::writeToLog(errorMsg);
        // Continue anyway, as gamepad might still work without sensors
    }
    else
    {
        juce::Logger::writeToLog("SDL sensor subsystem initialized successfully");
    }
    
    sdlInitialized = true;
    
    // Look for connected gamepads
    updateGamepadStates();
    
    return true;
}

void GamepadManager::cleanupSDL()
{
    // Close all open gamepads
    for (size_t i = 0; i < MAX_GAMEPADS; ++i)
    {
        if (sdlGamepads[i] != nullptr)
        {
            SDL_CloseGamepad(sdlGamepads[i]);
            sdlGamepads[i] = nullptr;
        }
    }
    
    // Quit SDL subsystems if it was initialized
    if (sdlInitialized)
    {
        SDL_Quit();
        sdlInitialized = false;
    }
}

void GamepadManager::timerCallback()
{
    updateGamepadStates();
}

void GamepadManager::updateGamepadStates()
{
    if (!sdlInitialized)
    {
        juce::Logger::writeToLog("DEBUG: SDL not initialized in updateGamepadStates");
        return;
    }
    
    // Process SDL events (important for device hot-plugging)
    handleSDLEvents();
    
    bool stateChanged = false;
    
    // Update states of connected gamepads
    for (size_t i = 0; i < MAX_GAMEPADS; ++i)
    {
        if (sdlGamepads[i] != nullptr)
        {
            // Update gyroscope data if enabled
            if (gamepadStates[i].gyroscope.enabled)
            {
                // Get the current joystick instance to verify it's still valid
                SDL_Joystick* joystick = SDL_GetGamepadJoystick(sdlGamepads[i]);
                if (joystick == nullptr)
                {
                    // Joystick became invalid, disable gyroscope
                    gamepadStates[i].gyroscope.enabled = false;
                    juce::Logger::writeToLog("Gyroscope disabled - invalid joystick handle");
                    continue;
                }

                // Verify sensor is still enabled
                if (!SDL_GamepadSensorEnabled(sdlGamepads[i], SDL_SENSOR_GYRO))
                {
                    // Try to re-enable the sensor
                    int result = SDL_SetGamepadSensorEnabled(sdlGamepads[i], SDL_SENSOR_GYRO, true);
                    if (result != 0)
                    {
                        gamepadStates[i].gyroscope.enabled = false;
                        juce::Logger::writeToLog("Failed to re-enable gyroscope: " + juce::String(SDL_GetError()));
                    }
                    else
                    {
                        juce::Logger::writeToLog("Re-enabled gyroscope on polling check");
                    }
                }
            }
            
            // Update axes
            for (size_t axis = 0; axis < MAX_AXES; ++axis)
            {
                SDL_GamepadAxis sdlAxis = SDL_GAMEPAD_AXIS_INVALID;
                
                // Map our axes indices to SDL axes
                switch (axis)
                {
                    case 0: sdlAxis = SDL_GAMEPAD_AXIS_LEFTX; break;
                    case 1: sdlAxis = SDL_GAMEPAD_AXIS_LEFTY; break;
                    case 2: sdlAxis = SDL_GAMEPAD_AXIS_RIGHTX; break;
                    case 3: sdlAxis = SDL_GAMEPAD_AXIS_RIGHTY; break;
                    case 4: sdlAxis = SDL_GAMEPAD_AXIS_LEFT_TRIGGER; break;
                    case 5: sdlAxis = SDL_GAMEPAD_AXIS_RIGHT_TRIGGER; break;
                    default: break;
                }
                
                if (sdlAxis != SDL_GAMEPAD_AXIS_INVALID)
                {
                    // SDL axes range from -32768 to 32767, normalize to -1.0 to 1.0
                    float axisValue = SDL_GetGamepadAxis(sdlGamepads[i], sdlAxis) / 32767.0f;
                    
                    // Apply a small deadzone
                    if (std::abs(axisValue) < 0.1f)
                        axisValue = 0.0f;
                    
                    // Check if value has changed
                    if (std::abs(gamepadStates[i].axes[axis] - axisValue) > 0.01f)
                    {
                        gamepadStates[i].axes[axis] = axisValue;
                        stateChanged = true;
                    }
                }
            }
            
            // Update buttons
            for (size_t button = 0; button < MAX_BUTTONS; ++button)
            {
                SDL_GamepadButton sdlButton = SDL_GAMEPAD_BUTTON_INVALID;
                
                // Map our button indices to SDL buttons
                switch (button)
                {
                    case 0: sdlButton = SDL_GAMEPAD_BUTTON_SOUTH; break;  // A
                    case 1: sdlButton = SDL_GAMEPAD_BUTTON_EAST; break;   // B
                    case 2: sdlButton = SDL_GAMEPAD_BUTTON_WEST; break;   // X
                    case 3: sdlButton = SDL_GAMEPAD_BUTTON_NORTH; break;  // Y
                    case 4: sdlButton = SDL_GAMEPAD_BUTTON_BACK; break;
                    case 5: sdlButton = SDL_GAMEPAD_BUTTON_GUIDE; break;
                    case 6: sdlButton = SDL_GAMEPAD_BUTTON_START; break;
                    case 7: sdlButton = SDL_GAMEPAD_BUTTON_LEFT_STICK; break;
                    case 8: sdlButton = SDL_GAMEPAD_BUTTON_RIGHT_STICK; break;
                    case 9: sdlButton = SDL_GAMEPAD_BUTTON_LEFT_SHOULDER; break;
                    case 10: sdlButton = SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER; break;
                    case 11: sdlButton = SDL_GAMEPAD_BUTTON_DPAD_UP; break;
                    case 12: sdlButton = SDL_GAMEPAD_BUTTON_DPAD_DOWN; break;
                    case 13: sdlButton = SDL_GAMEPAD_BUTTON_DPAD_LEFT; break;
                    case 14: sdlButton = SDL_GAMEPAD_BUTTON_DPAD_RIGHT; break;
                    default: break;
                }
                
                if (sdlButton != SDL_GAMEPAD_BUTTON_INVALID)
                {
                    bool buttonState = SDL_GetGamepadButton(sdlGamepads[i], sdlButton) == 1;
                    
                    // Check if state has changed
                    if (gamepadStates[i].buttons[button] != buttonState)
                    {
                        gamepadStates[i].buttons[button] = buttonState;
                        stateChanged = true;
                    }
                }
            }
            
            // Check touchpad button (it's a separate button in SDL)
            bool touchpadPressed = SDL_GetGamepadButton(sdlGamepads[i], SDL_GAMEPAD_BUTTON_TOUCHPAD) == 1;
            if (gamepadStates[i].touchpad.pressed != touchpadPressed)
            {
                gamepadStates[i].touchpad.pressed = touchpadPressed;
                stateChanged = true;
            }
        }
    }
    
    // Notify callbacks if any state changed
    if (stateChanged)
    {
        for (auto& callback : stateChangeCallbacks)
            callback();
    }
}

void GamepadManager::handleSDLEvents()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_EVENT_GAMEPAD_ADDED)
        {
            // Immediately handle the new gamepad connection
            SDL_JoystickID deviceId = event.gdevice.which;
            juce::Logger::writeToLog("DEBUG: Gamepad ADDED event received - event ID: " + juce::String(deviceId));
            
            // Log currently connected gamepads
            for (size_t j = 0; j < MAX_GAMEPADS; ++j) {
                if (sdlGamepads[j] != nullptr) {
                    juce::Logger::writeToLog("DEBUG: Existing gamepad at slot " + juce::String(j) + 
                                           ": " + gamepadStates[j].name + 
                                           " (ID: " + juce::String(gamepadStates[j].deviceId) + ")");
                }
            }
            
            // Find an empty slot for the new gamepad
            for (size_t i = 0; i < MAX_GAMEPADS; ++i)
            {
                if (sdlGamepads[i] == nullptr)
                {
                    juce::Logger::writeToLog("DEBUG: Found empty slot at index " + juce::String(i));
                    
                    // Check if it's actually a gamepad
                    bool isGamepad = SDL_IsGamepad(deviceId);
                    juce::Logger::writeToLog("DEBUG: Is device a gamepad? " + juce::String(isGamepad ? "Yes" : "No"));
                    
                    if (isGamepad)
                    {
                        sdlGamepads[i] = SDL_OpenGamepad(deviceId);
                        if (sdlGamepads[i] != nullptr)
                        {
                            gamepadStates[i].connected = true;
                            gamepadStates[i].name = SDL_GetGamepadName(sdlGamepads[i]);
                            gamepadStates[i].deviceId = deviceId;
                            
                            juce::Logger::writeToLog("DEBUG: Successfully opened gamepad at slot " + juce::String(i) + 
                                                   "\n - Name: " + gamepadStates[i].name + 
                                                   "\n - Device ID: " + juce::String(gamepadStates[i].deviceId) +
                                                   "\n - SDL Instance ID: " + juce::String(SDL_GetGamepadID(sdlGamepads[i])));

                            // Enable gyroscope if available
                            bool hasSensor = SDL_GamepadHasSensor(sdlGamepads[i], SDL_SENSOR_GYRO);
                            juce::Logger::writeToLog("Gyroscope support check for " + gamepadStates[i].name + ": " + (hasSensor ? "Supported" : "Not supported"));
                            
                            if (hasSensor)
                            {
                                // Log sensor capabilities
                                float data_rate = SDL_GetGamepadSensorDataRate(sdlGamepads[i], SDL_SENSOR_GYRO);
                                juce::Logger::writeToLog("Gyroscope data rate: " + juce::String(data_rate) + " Hz");
                                
                                // Try to enable the sensor
                                int result = SDL_SetGamepadSensorEnabled(sdlGamepads[i], SDL_SENSOR_GYRO, true);
                                juce::Logger::writeToLog("Enable sensor result: " + juce::String(result));
                                
                                // Check if sensor is actually enabled
                                if (SDL_GamepadSensorEnabled(sdlGamepads[i], SDL_SENSOR_GYRO))
                                {
                                    gamepadStates[i].gyroscope.enabled = true;
                                    juce::Logger::writeToLog("Gyroscope confirmed enabled for: " + gamepadStates[i].name);
                                    
                                    // Update device ID to match joystick ID for more reliable event matching
                                    SDL_Joystick* joystick = SDL_GetGamepadJoystick(sdlGamepads[i]);
                                    if (joystick != nullptr)
                                    {
                                        SDL_JoystickID joyId = SDL_GetJoystickID(joystick);
                                        gamepadStates[i].deviceId = joyId;
                                        juce::Logger::writeToLog("Updated device ID to joystick ID: " + juce::String(joyId));
                                    }
                                    
                                    // Log that we're using event-driven gyroscope data
                                    juce::Logger::writeToLog("Using event-driven gyroscope updates for " + gamepadStates[i].name);
                                }
                                else
                                {
                                    juce::Logger::writeToLog("Failed to enable gyroscope (state check failed): " + juce::String(SDL_GetError()));
                                }
                            }

                            // Check for accelerometer support
                            bool hasAccel = SDL_GamepadHasSensor(sdlGamepads[i], SDL_SENSOR_ACCEL);
                            juce::Logger::writeToLog("Accelerometer support check for " + gamepadStates[i].name + ": " + (hasAccel ? "Supported" : "Not supported"));
                            
                            if (hasAccel)
                            {
                                // Log accelerometer capabilities
                                float data_rate = SDL_GetGamepadSensorDataRate(sdlGamepads[i], SDL_SENSOR_ACCEL);
                                juce::Logger::writeToLog("Accelerometer data rate: " + juce::String(data_rate) + " Hz");
                                
                                // Try to enable the sensor
                                int result = SDL_SetGamepadSensorEnabled(sdlGamepads[i], SDL_SENSOR_ACCEL, true);
                                juce::Logger::writeToLog("Enable accelerometer result: " + juce::String(result));
                                
                                // Check if sensor is actually enabled
                                if (SDL_GamepadSensorEnabled(sdlGamepads[i], SDL_SENSOR_ACCEL))
                                {
                                    gamepadStates[i].accelerometer.enabled = true;
                                    juce::Logger::writeToLog("Accelerometer confirmed enabled for: " + gamepadStates[i].name);
                                }
                            }
                            
                            // Notify callbacks of the new connection
                            for (auto& callback : stateChangeCallbacks)
                                callback();
                            
                            break;
                        }
                        else
                        {
                            juce::Logger::writeToLog("DEBUG: Failed to open gamepad: " + juce::String(SDL_GetError()));
                        }
                    }
                }
            }
        }
        else if (event.type == SDL_EVENT_GAMEPAD_REMOVED)
        {
            // Find which gamepad was disconnected
            for (size_t i = 0; i < MAX_GAMEPADS; ++i)
            {
                if (sdlGamepads[i] != nullptr && 
                    (SDL_GetGamepadID(sdlGamepads[i]) == event.gdevice.which || 
                     gamepadStates[i].deviceId == event.gdevice.which))
                {
                    juce::Logger::writeToLog("Gamepad disconnected - event ID: " + juce::String(event.gdevice.which) + 
                                          ", device ID: " + juce::String(gamepadStates[i].deviceId));
                    SDL_CloseGamepad(sdlGamepads[i]);
                    sdlGamepads[i] = nullptr;
                    gamepadStates[i].connected = false;
                    juce::Logger::writeToLog("Gamepad disconnected: " + gamepadStates[i].name);
                    gamepadStates[i].name = "";
                    
                    // Reset all state values
                    for (auto& axis : gamepadStates[i].axes)
                        axis = 0.0f;
                    
                    for (auto& button : gamepadStates[i].buttons)
                        button = false;
                    
                    // Reset touchpad state
                    gamepadStates[i].touchpad.touched = false;
                    gamepadStates[i].touchpad.pressed = false;
                    gamepadStates[i].touchpad.x = 0.0f;
                    gamepadStates[i].touchpad.y = 0.0f;
                    gamepadStates[i].touchpad.pressure = 0.0f;
                    
                    // Notify callbacks
                    for (auto& callback : stateChangeCallbacks)
                        callback();
                    
                    break;
                }
            }
        }
        // Handle sensor update events
        else if (event.type == SDL_EVENT_GAMEPAD_SENSOR_UPDATE)
        {
            SDL_JoystickID gamepadId = event.gsensor.which;
            
            // Find which gamepad this sensor event belongs to
            for (size_t i = 0; i < MAX_GAMEPADS; ++i)
            {
                if (sdlGamepads[i] != nullptr && 
                    // Check against both stored deviceId and current ID from SDL
                    (gamepadStates[i].deviceId == gamepadId || 
                     SDL_GetGamepadID(sdlGamepads[i]) == gamepadId))
                {
                    // Make sure we have the correct ID stored
                    if (gamepadStates[i].deviceId != gamepadId)
                    {
                        juce::Logger::writeToLog("Updating device ID from " + juce::String(gamepadStates[i].deviceId) + 
                                               " to " + juce::String(gamepadId));
                        gamepadStates[i].deviceId = gamepadId;
                    }
                    
                    bool stateChanged = false;
                    static int logCounter = 0;  // Static counter to limit log frequency
                    
                    // This is a gyroscope event
                    if (event.gsensor.sensor == SDL_SENSOR_GYRO)
                    {
                        // Check if values have changed significantly (apply small threshold)
                        if (std::abs(gamepadStates[i].gyroscope.x - event.gsensor.data[0]) > 0.01f)
                        {
                            gamepadStates[i].gyroscope.x = event.gsensor.data[0];
                            stateChanged = true;
                        }
                        if (std::abs(gamepadStates[i].gyroscope.y - event.gsensor.data[1]) > 0.01f)
                        {
                            gamepadStates[i].gyroscope.y = event.gsensor.data[1];
                            stateChanged = true;
                        }
                        if (std::abs(gamepadStates[i].gyroscope.z - event.gsensor.data[2]) > 0.01f)
                        {
                            gamepadStates[i].gyroscope.z = event.gsensor.data[2];
                            stateChanged = true;
                        }
                        
                        if (stateChanged)
                        {
                            // Indicate that the gyroscope is working
                            gamepadStates[i].gyroscope.enabled = true;
                            
                            // Log gyro data occasionally to avoid flooding
                            if (++logCounter >= 30)  // Log every ~30th change
                            {
                                logCounter = 0;
                                juce::Logger::writeToLog(juce::String::formatted("Gyro data (event-driven) - X: %.2f, Y: %.2f, Z: %.2f, timestamp: %llu",
                                                                               event.gsensor.data[0], 
                                                                               event.gsensor.data[1], 
                                                                               event.gsensor.data[2],
                                                                               event.gsensor.sensor_timestamp));
                            }
                        }
                    }
                    // This is an accelerometer event
                    else if (event.gsensor.sensor == SDL_SENSOR_ACCEL)
                    {
                        // Check if values have changed significantly (apply small threshold)
                        if (std::abs(gamepadStates[i].accelerometer.x - event.gsensor.data[0]) > 0.01f)
                        {
                            gamepadStates[i].accelerometer.x = event.gsensor.data[0];
                            stateChanged = true;
                        }
                        if (std::abs(gamepadStates[i].accelerometer.y - event.gsensor.data[1]) > 0.01f)
                        {
                            gamepadStates[i].accelerometer.y = event.gsensor.data[1];
                            stateChanged = true;
                        }
                        if (std::abs(gamepadStates[i].accelerometer.z - event.gsensor.data[2]) > 0.01f)
                        {
                            gamepadStates[i].accelerometer.z = event.gsensor.data[2];
                            stateChanged = true;
                        }
                        
                        if (stateChanged)
                        {
                            // Indicate that the accelerometer is working
                            gamepadStates[i].accelerometer.enabled = true;
                            
                            // Log accelerometer data occasionally to avoid flooding
                            if (++logCounter >= 30)  // Log every ~30th change
                            {
                                logCounter = 0;
                                juce::Logger::writeToLog(juce::String::formatted("Accel data (event-driven) - X: %.2f, Y: %.2f, Z: %.2f, timestamp: %llu",
                                                                               event.gsensor.data[0], 
                                                                               event.gsensor.data[1], 
                                                                               event.gsensor.data[2],
                                                                               event.gsensor.sensor_timestamp));
                            }
                        }
                    }
                    
                    // Notify callbacks if state changed
                    if (stateChanged)
                    {
                        for (auto& callback : stateChangeCallbacks)
                            callback();
                    }
                    
                    break;
                }
            }
        }
        // Handle touchpad events for supported controllers (e.g. PlayStation DualSense)
        else if (event.type == SDL_EVENT_GAMEPAD_TOUCHPAD_DOWN || 
                 event.type == SDL_EVENT_GAMEPAD_TOUCHPAD_MOTION ||
                 event.type == SDL_EVENT_GAMEPAD_TOUCHPAD_UP)
        {
            // Find which gamepad this touchpad event belongs to
            for (size_t i = 0; i < MAX_GAMEPADS; ++i)
            {
                if (sdlGamepads[i] != nullptr && 
                    // Check against both stored deviceId and current ID from SDL
                    (gamepadStates[i].deviceId == event.gtouchpad.which || 
                     SDL_GetGamepadID(sdlGamepads[i]) == event.gtouchpad.which))
                {
                    // Make sure we have the correct ID stored
                    if (gamepadStates[i].deviceId != event.gtouchpad.which)
                    {
                        juce::Logger::writeToLog("Updating device ID from " + juce::String(gamepadStates[i].deviceId) + 
                                               " to " + juce::String(event.gtouchpad.which) + " in touchpad event");
                        gamepadStates[i].deviceId = event.gtouchpad.which;
                    }
                    
                    bool stateChanged = false;
                    
                    // Update touchpad state based on event type
                    if (event.type == SDL_EVENT_GAMEPAD_TOUCHPAD_DOWN)
                    {
                        gamepadStates[i].touchpad.touched = true;
                        gamepadStates[i].touchpad.x = event.gtouchpad.x;
                        gamepadStates[i].touchpad.y = event.gtouchpad.y;
                        gamepadStates[i].touchpad.pressure = event.gtouchpad.pressure;
                        stateChanged = true;
                    }
                    else if (event.type == SDL_EVENT_GAMEPAD_TOUCHPAD_MOTION)
                    {
                        gamepadStates[i].touchpad.x = event.gtouchpad.x;
                        gamepadStates[i].touchpad.y = event.gtouchpad.y;
                        gamepadStates[i].touchpad.pressure = event.gtouchpad.pressure;
                        stateChanged = true;
                    }
                    else if (event.type == SDL_EVENT_GAMEPAD_TOUCHPAD_UP)
                    {
                        gamepadStates[i].touchpad.touched = false;
                        gamepadStates[i].touchpad.pressure = 0.0f;
                        stateChanged = true;
                    }
                    
                    // Notify callbacks if state changed
                    if (stateChanged)
                    {
                        for (auto& callback : stateChangeCallbacks)
                            callback();
                    }
                    
                    break;
                }
            }
        }
    }
}

const GamepadManager::GamepadState& GamepadManager::getGamepadState(int index) const
{
    // Ensure index is in range
    jassert(index >= 0 && index < MAX_GAMEPADS);
    return gamepadStates[static_cast<size_t>(index)];
}

int GamepadManager::getNumConnectedGamepads() const
{
    int count = 0;
    for (const auto& state : gamepadStates)
    {
        if (state.connected)
            count++;
    }
    return count;
}

bool GamepadManager::isGamepadConnected(int index) const
{
    if (index >= 0 && index < MAX_GAMEPADS)
        return gamepadStates[static_cast<size_t>(index)].connected;
    return false;
}

void GamepadManager::addStateChangeCallback(StateChangeCallback callback)
{
    stateChangeCallbacks.push_back(std::move(callback));
} 