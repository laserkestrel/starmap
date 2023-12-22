cmake_minimum_required(VERSION 3.16)
project(starmap3 LANGUAGES CXX)

# Set the build type to Release/Debug
set(CMAKE_BUILD_TYPE Debug)

# Add debug symbols for Debug build type, and enable debug console msgs
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options("-g")
    add_definitions(-D_DEBUG)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
option(BUILD_SHARED_LIBS "Build shared libraries" ON)

include(FetchContent)
FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 2.6.x)
FetchContent_MakeAvailable(SFML)

file(GLOB SOURCES "src/*.cpp")
add_executable(starmap3 ${SOURCES})
target_link_libraries(starmap3 PRIVATE sfml-graphics)
target_compile_features(starmap3 PRIVATE cxx_std_17)

if(WIN32)
    add_custom_command(
        TARGET starmap3
        COMMENT "Copy OpenAL DLL"
        PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${SFML_SOURCE_DIR}/extlibs/bin/$<IF:$<EQUAL:${CMAKE_SIZEOF_VOID_P},8>,x64,x86>/openal32.dll $<TARGET_FILE_DIR:starmap3>
        VERBATIM)
endif()

install(TARGETS starmap3)
