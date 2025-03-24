cmake_minimum_required(VERSION 3.25)

# Set CMake policy to suppress warnings
cmake_policy(SET CMP0135 NEW)

# This tells cmake we have goodies in the /cmake folder
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Include FetchContent for dependency management
include(FetchContent)

# Add a version manually if PamplejuceVersion is not available
set(CURRENT_VERSION "0.0.1")
set(MAJOR_VERSION "0")
set(MINOR_VERSION "0")
set(PATCH_VERSION "1")

# Add debug symbols configuration for Release builds
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -g")
set(CMAKE_COMPILE_OPTIONS_IPO "${CMAKE_COMPILE_OPTIONS_IPO} -g")

# Configures universal binaries and decides which version of macOS to support
include(PamplejuceMacOS)

# Couple tweaks that IMO should be JUCE defaults
include(JUCEDefaults)

# Project name and details
set(PROJECT_NAME "GamepadMIDI")
set(PRODUCT_NAME "Gamepad MIDI")
set(COMPANY_NAME "PoundingSystems")
set(BUNDLE_ID "com.poundingSystems.gamepadmidi")

# Enable C language for SDL3
enable_language(C)

project(${PROJECT_NAME} VERSION ${CURRENT_VERSION} LANGUAGES C CXX)

# macOS specific configuration
if(APPLE)
    # Define that we're on macOS for conditional compilation
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DJUCE_MAC=1")
    
    # Critical: explicitly disable App Sandbox for MIDI virtual devices
    set(CMAKE_XCODE_ATTRIBUTE_ENABLE_APP_SANDBOX "NO")
    
    # Make sure we use the hardened runtime (required for notarization)
    set(CMAKE_XCODE_ATTRIBUTE_ENABLE_HARDENED_RUNTIME "YES")
    
    # Force proper code signing for MIDI functionality
    # For distribution, use "Developer ID Application" instead of "Apple Development"
    set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "Developer ID Application")
    # You MUST set your development team ID here for distribution
    set(CMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM "")  # Replace with your Team ID from Apple Developer Account
    set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED "YES")
    set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED "YES")
    set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGN_STYLE "Manual")  # Use Manual for Developer ID signing
    
    # Allow unsigned libraries for MIDI drivers
    set(CMAKE_XCODE_ATTRIBUTE_OTHER_CODE_SIGN_FLAGS "--deep --options=runtime")
endif()

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

# JUCE is setup as a submodule in the /JUCE folder
add_subdirectory(JUCE)
add_subdirectory(modules/melatonin_inspector)
# Add SDL3 for gamepad support
set(SDL_FORCE_STATIC OFF)
set(SDL_SHARED ON)
set(SDL_STATIC OFF)
set(SDL_OSX_ARCHITECTURES "${CMAKE_OSX_ARCHITECTURES}" CACHE STRING "Build architectures for macOS" FORCE)

FetchContent_Declare(
    SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-3.2.8
)
FetchContent_MakeAvailable(SDL3)

# Create a SharedCode interface library
add_library(SharedCode INTERFACE)

# Create the standalone application target
juce_add_gui_app(${PROJECT_NAME}
    VERSION ${CURRENT_VERSION}
    ICON_BIG "${CMAKE_CURRENT_SOURCE_DIR}/packaging/icon.png"
    COMPANY_NAME "${COMPANY_NAME}"
    BUNDLE_ID "${BUNDLE_ID}"
    PRODUCT_NAME "${PRODUCT_NAME}"
    MICROPHONE_PERMISSION_ENABLED TRUE
    MICROPHONE_PERMISSION_TEXT "This app requires microphone access to enable MIDI device creation."
    DOCUMENT_BROWSER_ENABLED FALSE
    HARDENED_RUNTIME_ENABLED TRUE
    APP_SANDBOX_ENABLED FALSE
    DOCUMENT_EXTENSIONS ""
    PUSH_NOTIFICATIONS_ENABLED FALSE
    FILE_SHARING_ENABLED FALSE
    PLIST_TO_MERGE "${CMAKE_CURRENT_SOURCE_DIR}/packaging/Info.plist"
)

# Set entitlements for macOS
if(APPLE)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        XCODE_ATTRIBUTE_CODE_SIGN_ENTITLEMENTS "${CMAKE_CURRENT_SOURCE_DIR}/packaging/GamepadMIDI.entitlements"
    )
endif()

# Add source files
file(GLOB_RECURSE SourceFiles CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/source/*.h")
target_sources(SharedCode INTERFACE ${SourceFiles})

# Add JUCE module header path and source
target_include_directories(SharedCode 
    INTERFACE 
        ${CMAKE_CURRENT_SOURCE_DIR}/source
        ${JUCE_MODULES_DIR}
)

# JUCE modules and compile definitions
target_compile_definitions(SharedCode
    INTERFACE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_APPLICATION_NAME_STRING="${PRODUCT_NAME}"
        JUCE_APPLICATION_VERSION_STRING="${CURRENT_VERSION}"
        JUCE_DISPLAY_SPLASH_SCREEN=0
        JUCE_MODAL_LOOPS_PERMITTED=1
        JUCE_STANDALONE_APPLICATION=1
        $<$<CONFIG:Debug>:DEBUG=1>
        $<$<CONFIG:Debug>:_DEBUG=1>
        $<$<NOT:$<CONFIG:Debug>>:NDEBUG=1>
        CMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE}"
        VERSION="${CURRENT_VERSION}"
)

# Link SharedCode with JUCE modules
target_link_libraries(SharedCode
    INTERFACE
        SDL3::SDL3
        juce::juce_core
        juce::juce_data_structures
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_utils
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
        melatonin_inspector
        Assets
)

# Link the main target with SharedCode
target_link_libraries(${PROJECT_NAME} 
    PRIVATE 
        SharedCode
        melatonin_inspector
        Assets
)

# Set C++20 standard
target_compile_features(SharedCode INTERFACE cxx_std_20)

# Handle SDL3 dependencies for macOS
if(APPLE)
    # Critical: explicitly disable App Sandbox for MIDI virtual devices
    set(CMAKE_XCODE_ATTRIBUTE_ENABLE_APP_SANDBOX "NO")
    
    # Make sure we use the hardened runtime (required for notarization)
    set(CMAKE_XCODE_ATTRIBUTE_ENABLE_HARDENED_RUNTIME "YES")
    
    # Force proper code signing for MIDI functionality
    set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "Developer ID Application")
    set(CMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM "")  # Replace with your Team ID from Apple Developer Account
    set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED "YES")
    set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED "YES")
    set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGN_STYLE "Manual")  # Use Manual for Developer ID signing
    
    # Allow unsigned libraries for MIDI drivers
    set(CMAKE_XCODE_ATTRIBUTE_OTHER_CODE_SIGN_FLAGS "--deep --options=runtime")

    # Add post-build command to copy SDL3 into the app bundle
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${PROJECT_NAME}>/../Frameworks"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:SDL3-shared>"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/../Frameworks/"
        COMMAND install_name_tool -change "@rpath/libSDL3.0.dylib" 
            "@executable_path/../Frameworks/libSDL3.0.dylib"
            "$<TARGET_FILE:${PROJECT_NAME}>"
    )
endif()

juce_add_binary_data(Assets 
    SOURCES
        assets/images/PoundingSystemsLogo.png
)

add_custom_command(TARGET "${PROJECT_NAME}" POST_BUILD
    COMMAND codesign --force --sign - --entitlements "${CMAKE_CURRENT_SOURCE_DIR}/packaging/GamepadMIDI.entitlements" "$<TARGET_FILE_DIR:${PROJECT_NAME}>/../.."
    COMMENT "Signing app with entitlements")