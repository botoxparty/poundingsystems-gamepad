#include "helpers/test_helpers.h"
#include <PluginProcessor.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <catch2/catch_all.hpp>
#include "../source/GamepadManager.h"

TEST_CASE ("one is equal to one", "[dummy]")
{
    REQUIRE (1 == 1);
}

TEST_CASE ("Plugin instance", "[instance]")
{
    PluginProcessor testPlugin;

    SECTION ("name")
    {
        CHECK_THAT (testPlugin.getName().toStdString(),
            Catch::Matchers::Equals ("Pamplejuce Demo"));
    }
}


#ifdef PAMPLEJUCE_IPP
    #include <ipp.h>

TEST_CASE ("IPP version", "[ipp]")
{
    CHECK_THAT (ippsGetLibVersion()->Version, Catch::Matchers::Equals ("2022.0.0 (r0x131e93b0)"));
}
#endif

// Very basic test just to make sure the test framework builds
TEST_CASE("Plugin basics", "[plugin]") 
{
    REQUIRE(1 == 1);
}

TEST_CASE("GamepadManager Basic Tests", "[gamepad]")
{
    GamepadManager manager;
    
    SECTION("Initial State")
    {
        // Check initial state
        REQUIRE(manager.getNumConnectedGamepads() == 0);
        
        // Check invalid index returns not connected
        REQUIRE_FALSE(manager.isGamepadConnected(-1));
        REQUIRE_FALSE(manager.isGamepadConnected(GamepadManager::MAX_GAMEPADS));
        
        // Check first gamepad state
        const auto& state = manager.getGamepadState(0);
        REQUIRE_FALSE(state.connected);
        REQUIRE(state.name.isEmpty());
        
        // Check all axes are zero
        for (const auto& axis : state.axes)
        {
            REQUIRE(axis == 0.0f);
        }
        
        // Check all buttons are not pressed
        for (const auto& button : state.buttons)
        {
            REQUIRE_FALSE(button);
        }
    }
}
