cmake_minimum_required(VERSION 3.16)
project(starmap3 LANGUAGES CXX)

# ------------------------------------------------------------
# Global project settings
# ------------------------------------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Put executables in a predictable place
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Ensure consistent MSVC runtime
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# ------------------------------------------------------------
# Dependencies
# ------------------------------------------------------------
include(FetchContent)

# Fetch SFML 2.6 from GitHub and build
FetchContent_Declare(
    SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 2.6.x
)
set(BUILD_SHARED_LIBS ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(SFML)

# nlohmann-json via vcpkg
find_package(nlohmann_json CONFIG REQUIRED)

# ------------------------------------------------------------
# Target
# ------------------------------------------------------------
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

# Link libraries
target_link_libraries(starmap3
    PRIVATE
        sfml-graphics
        sfml-window
        sfml-system
        nlohmann_json::nlohmann_json
)

# Optional: explicit Debug macro
target_compile_definitions(starmap3
    PRIVATE
        $<$<CONFIG:Debug>:_DEBUG>
)

# ------------------------------------------------------------
# Install
# ------------------------------------------------------------
install(TARGETS starmap3)
