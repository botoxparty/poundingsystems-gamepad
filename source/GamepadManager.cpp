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
        return;
    
    // Process SDL events (important for device hot-plugging)
    handleSDLEvents();
    
    // Check for new gamepads
    for (size_t i = 0; i < MAX_GAMEPADS; ++i)
    {
        if (sdlGamepads[i] == nullptr)
        {
            // Try to open the gamepad using device index
            if (SDL_IsGamepad(static_cast<int>(i)))
            {
                sdlGamepads[i] = SDL_OpenGamepad(static_cast<int>(i));
                if (sdlGamepads[i] != nullptr)
                {
                    gamepadStates[i].connected = true;
                    gamepadStates[i].name = SDL_GetGamepadName(sdlGamepads[i]);
                    gamepadStates[i].deviceId = SDL_GetGamepadID(sdlGamepads[i]);
                    SDL_Joystick* joystick = SDL_GetGamepadJoystick(sdlGamepads[i]);
                    
                    // Add detailed joystick information
                    if (joystick != nullptr)
                    {
                        SDL_JoystickID joyId = SDL_GetJoystickID(joystick);
                        juce::Logger::writeToLog("Detailed joystick info:");
                        juce::Logger::writeToLog(" - Joystick instance ID: " + juce::String(joyId));
                        juce::Logger::writeToLog(" - Is game gamepad: " + juce::String((int)SDL_IsGamepad(joyId)));
                    }
                    
                    juce::Logger::writeToLog("Gamepad details:");
                    juce::Logger::writeToLog(" - Name: " + gamepadStates[i].name);
                    juce::Logger::writeToLog(" - Gamepad ID: " + juce::String(gamepadStates[i].deviceId));
                    juce::Logger::writeToLog(" - Joystick Instance ID: " + juce::String(SDL_GetGamepadID(sdlGamepads[i])));
                    juce::Logger::writeToLog(" - Index used: " + juce::String(i));
                    
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
                            
                            // Try an immediate sensor read to verify
                            float testData[3];
                            int readResult = SDL_GetGamepadSensorData(sdlGamepads[i], SDL_SENSOR_GYRO, testData, 3);
                            juce::Logger::writeToLog("Initial sensor read result: " + juce::String(readResult) + 
                                                   " (Error: " + juce::String(SDL_GetError()) + ")");
                        }
                        else
                        {
                            juce::Logger::writeToLog("Failed to enable gyroscope (state check failed): " + juce::String(SDL_GetError()));
                        }
                    }
                    
                    // Notify callbacks
                    for (auto& callback : stateChangeCallbacks)
                        callback();
                }
            }
        }
    }
    
    // Update states of connected gamepads
    bool stateChanged = false;
    
    for (size_t i = 0; i < MAX_GAMEPADS; ++i)
    {
        if (sdlGamepads[i] != nullptr)
        {
            // Update gyroscope data if enabled
            if (gamepadStates[i].gyroscope.enabled)
            {
                float gyroData[3] = {0.0f, 0.0f, 0.0f};  // Initialize to zero
                
                // Get the current joystick instance to verify it's still valid
                SDL_Joystick* joystick = SDL_GetGamepadJoystick(sdlGamepads[i]);
                if (joystick == nullptr)
                {
                    // Joystick became invalid, disable gyroscope
                    gamepadStates[i].gyroscope.enabled = false;
                    juce::Logger::writeToLog("Gyroscope disabled - invalid joystick handle");
                    continue;
                }

                // Verify sensor is still enabled before reading
                if (!SDL_GamepadSensorEnabled(sdlGamepads[i], SDL_SENSOR_GYRO))
                {
                    // Try to re-enable the sensor
                    if (SDL_SetGamepadSensorEnabled(sdlGamepads[i], SDL_SENSOR_GYRO, true) != 0)
                    {
                        gamepadStates[i].gyroscope.enabled = false;
                        juce::Logger::writeToLog("Failed to re-enable gyroscope: " + juce::String(SDL_GetError()));
                        continue;
                    }
                }
                
                // Try to read sensor data
                int readResult = SDL_GetGamepadSensorData(sdlGamepads[i], SDL_SENSOR_GYRO, gyroData, 3);
                if (readResult == 0)  // Success
                {
                    bool gyroChanged = false;
                    static int logCounter = 0;  // Static counter to limit log frequency
                    
                    // Check if values have changed significantly (apply small threshold)
                    if (std::abs(gamepadStates[i].gyroscope.x - gyroData[0]) > 0.01f)
                    {
                        gamepadStates[i].gyroscope.x = gyroData[0];
                        gyroChanged = true;
                    }
                    if (std::abs(gamepadStates[i].gyroscope.y - gyroData[1]) > 0.01f)
                    {
                        gamepadStates[i].gyroscope.y = gyroData[1];
                        gyroChanged = true;
                    }
                    if (std::abs(gamepadStates[i].gyroscope.z - gyroData[2]) > 0.01f)
                    {
                        gamepadStates[i].gyroscope.z = gyroData[2];
                        gyroChanged = true;
                    }
                    
                    if (gyroChanged)
                    {
                        stateChanged = true;
                        
                        // Log gyro data occasionally to avoid flooding
                        if (++logCounter >= 30)  // Log every ~30th change
                        {
                            logCounter = 0;
                            juce::Logger::writeToLog(juce::String::formatted("Gyro data - X: %.2f, Y: %.2f, Z: %.2f",
                                                                           gyroData[0], gyroData[1], gyroData[2]));
                        }
                    }
                }
                else
                {
                    static bool errorLogged = false;
                    if (!errorLogged)
                    {
                        SDL_JoystickID joyId = SDL_GetGamepadID(sdlGamepads[i]);
                        SDL_Joystick* joystick = SDL_GetGamepadJoystick(sdlGamepads[i]);
                        
                        juce::Logger::writeToLog("Gyroscope read error details:");
                        juce::Logger::writeToLog(" - Error code: " + juce::String(readResult));
                        juce::Logger::writeToLog(" - SDL Error: " + juce::String(SDL_GetError()));
                        juce::Logger::writeToLog(" - Gamepad ID: " + juce::String(joyId));
                        if (joystick)
                        {
                            juce::Logger::writeToLog(" - Joystick ID: " + juce::String(SDL_GetJoystickID(joystick)));
                        }
                        juce::Logger::writeToLog(" - Index: " + juce::String(i));
                        errorLogged = true;
                        
                        // Check if the gamepad is still valid
                        if (SDL_GetGamepadFromID(joyId) == nullptr)
                        {
                            juce::Logger::writeToLog("Gamepad no longer valid, disabling gyroscope");
                            gamepadStates[i].gyroscope.enabled = false;
                            continue;
                        }
                        
                        // Try to re-enable the sensor
                        if (SDL_SetGamepadSensorEnabled(sdlGamepads[i], SDL_SENSOR_GYRO, true) == 0)
                        {
                            juce::Logger::writeToLog("Re-enabled gyroscope after error");
                            errorLogged = false;  // Reset error log flag to catch any new errors
                        }
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
            // A new gamepad was connected, this will be handled in updateGamepadStates
        }
        else if (event.type == SDL_EVENT_GAMEPAD_REMOVED)
        {
            // Find which gamepad was disconnected
            for (size_t i = 0; i < MAX_GAMEPADS; ++i)
            {
                if (sdlGamepads[i] != nullptr && SDL_GetGamepadID(sdlGamepads[i]) == event.gdevice.which)
                {
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
        // Handle touchpad events for supported controllers (e.g. PlayStation DualSense)
        else if (event.type == SDL_EVENT_GAMEPAD_TOUCHPAD_DOWN || 
                 event.type == SDL_EVENT_GAMEPAD_TOUCHPAD_MOTION ||
                 event.type == SDL_EVENT_GAMEPAD_TOUCHPAD_UP)
        {
            // Find which gamepad this touchpad event belongs to
            for (size_t i = 0; i < MAX_GAMEPADS; ++i)
            {
                if (sdlGamepads[i] != nullptr && SDL_GetGamepadID(sdlGamepads[i]) == event.gtouchpad.which)
                {
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