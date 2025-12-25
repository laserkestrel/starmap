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

# ------------------------------------------------------------
# Dependencies (via vcpkg)
# ------------------------------------------------------------
# vcpkg toolchain is provided by the build system (CI / user)

find_package(SFML 2.6 CONFIG REQUIRED COMPONENTS graphics window system)
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

target_link_libraries(starmap3
    PRIVATE
        sfml-graphics
        sfml-window
        sfml-system
        nlohmann_json::nlohmann_json
)

# Optional but fine: explicit Debug macro (MSVC already defines _DEBUG)
target_compile_definitions(starmap3
    PRIVATE
        $<$<CONFIG:Debug>:_DEBUG>
)

# ------------------------------------------------------------
# Install
# ------------------------------------------------------------

install(TARGETS starmap3)
