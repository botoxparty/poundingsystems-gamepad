#pragma once

#include <juce_events/juce_events.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_sensor.h>
#include <SDL3/SDL_joystick.h>
#include <functional>
#include <array>
#include <vector>
#include <memory>

/**
 * GamepadManager class that handles initialization of SDL and gamepad input.
 * This class is responsible for detecting gamepads, reading their state,
 * and providing the state to other components of the application.
 */
class GamepadManager : private juce::Timer
{
public:
    // Maximum number of gamepads we'll support
    static constexpr int MAX_GAMEPADS = 4;
    
    // Maximum number of axes we'll track per gamepad
    static constexpr int MAX_AXES = 6;
    
    // Maximum number of buttons we'll track per gamepad
    static constexpr int MAX_BUTTONS = 15;
    
    struct GamepadState
    {
        bool connected = false;
        SDL_JoystickID deviceId = 0;  // Using 0 as sentinel value for uninitialized device
        std::array<float, MAX_AXES> axes = {0};       // Values from -1.0 to 1.0
        std::array<bool, MAX_BUTTONS> buttons = {false};
        juce::String name;
        
        // Touchpad support
        struct TouchpadState {
            bool touched = false;
            bool pressed = false;  // Touchpad button press state
            float x = 0.0f; // Normalized position (0.0 to 1.0)
            float y = 0.0f; // Normalized position (0.0 to 1.0)
            float pressure = 0.0f; // 0.0 to 1.0
        };
        TouchpadState touchpad;

        // Gyroscope support
        struct GyroscopeState {
            bool enabled = false;
            float x = 0.0f; // Rotation rate around X axis in radians/second
            float y = 0.0f; // Rotation rate around Y axis in radians/second
            float z = 0.0f; // Rotation rate around Z axis in radians/second
        };
        GyroscopeState gyroscope;
    };
    
    GamepadManager();
    ~GamepadManager() override;
    
    // Get the state of a specific gamepad
    const GamepadState& getGamepadState(int index) const;
    
    // Get the number of connected gamepads
    int getNumConnectedGamepads() const;
    
    // Check if gamepad at index is connected
    bool isGamepadConnected(int index) const;
    
    // Add a listener to be notified when gamepad state changes
    using StateChangeCallback = std::function<void()>;
    void addStateChangeCallback(StateChangeCallback callback);
    
    // Manually poll for gamepad state updates
    void updateGamepadStates();
    
private:
    // Initialize SDL and gamepad subsystem
    bool initSDL();
    
    // Clean up SDL resources
    void cleanupSDL();
    
    // Timer callback to poll gamepad state
    void timerCallback() override;
    
    // Handle SDL events
    void handleSDLEvents();
    
    // Array of gamepad states for all potential gamepads
    std::array<GamepadState, MAX_GAMEPADS> gamepadStates;
    
    // Array of SDL gamepad handles
    std::array<SDL_Gamepad*, MAX_GAMEPADS> sdlGamepads = {nullptr};
    
    // Flag to indicate if SDL has been successfully initialized
    bool sdlInitialized = false;
    
    // Vector of callbacks to notify when gamepad state changes
    std::vector<StateChangeCallback> stateChangeCallbacks;
}; 