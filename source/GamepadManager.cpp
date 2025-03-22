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
    // Initialize SDL with gamepad subsystem
    if (SDL_Init(SDL_INIT_GAMEPAD) < 0)
    {
        juce::String errorMsg = "SDL could not initialize! SDL Error: " + juce::String(SDL_GetError());
        juce::Logger::writeToLog(errorMsg);
        return false;
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
                    juce::Logger::writeToLog("Gamepad connected: " + gamepadStates[i].name);
                    
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