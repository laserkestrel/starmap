cmake_minimum_required(VERSION 3.16)
project(starmap3 LANGUAGES CXX)

# Output binaries to a predictable location
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Use modern C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Do NOT force build type (important for Windows / CI)
# Let the generator or CI choose Debug/Release

# Ensure consistent MSVC runtime (Debug vs Release)
if (MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# Export compile_commands.json (useful, harmless)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ---- Dependencies ----
include(FetchContent)

FetchContent_Declare(
    SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 2.6.x
)

# Build SFML as shared libs (default ON, but explicit is clearer)
set(BUILD_SHARED_LIBS ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(SFML)

# ---- Your executable ----
file(GLOB CONFIGURE_DEPENDS SOURCES "src/*.cpp")

add_executable(starmap3 ${SOURCES})

target_link_libraries(starmap3 PRIVATE sfml-graphics)

target_compile_features(starmap3 PRIVATE cxx_std_17)

# Define _DEBUG only for Debug builds, in a portable way
target_compile_definitions(starmap3 PRIVATE
    $<$<CONFIG:Debug>:_DEBUG>
)

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

# ---- Install rule (optional, but fine) ----
install(TARGETS starmap3)
