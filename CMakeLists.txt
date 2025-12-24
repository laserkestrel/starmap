cmake_minimum_required(VERSION 3.16)
project(starmap3 LANGUAGES CXX)

# Output binaries
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Use modern C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Ensure consistent MSVC runtime
if (MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# ---- Dependencies ----
include(FetchContent)

# SFML
FetchContent_Declare(
    SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 2.6.x
)
set(BUILD_SHARED_LIBS ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(SFML)

# nlohmann-json (via vcpkg)
find_package(nlohmann_json CONFIG REQUIRED)

# ---- Explicit sources ----
add_executable(starmap3
    src/GalaxyQuadTree.cpp
    src/GalaxyQuadTreeNode.cpp
    src/Game.cpp
    src/LoadConfig.cpp
    src/LoadCSVData.cpp
    src/Main.cpp
    src/Probe.cpp
    src/RenderSystem.cpp
    src/Star.cpp
    src/Utilities.cpp
)

# ---- Link libraries ----
target_link_libraries(starmap3 PRIVATE sfml-graphics nlohmann_json::nlohmann_json)
target_compile_features(starmap3 PRIVATE cxx_std_17)
target_compile_definitions(starmap3 PRIVATE $<$<CONFIG:Debug>:_DEBUG>)

# ---- Windows DLL handling ----
if (WIN32)
    add_custom_command(
        TARGET starmap3
        PRE_BUILD
        COMMENT "Copy OpenAL DLL"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${SFML_SOURCE_DIR}/extlibs/bin/$<IF:$<EQUAL:${CMAKE_SIZEOF_VOID_P},8>,x64,x86>/openal32.dll
            $<TARGET_FILE_DIR:starmap3>
    )
endif()

# Install target
install(TARGETS starmap3)
