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

# nlohmann/json -- LoadConfig.cpp includes <nlohmann/json.hpp>, so it is a real
# dependency of this target and belongs here rather than being assumed to exist on
# the system include path. Fetched from the packaged release archive (a few hundred
# KB) rather than the git repo, which carries a very large test-data tree.
FetchContent_Declare(
    nlohmann_json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
    URL_HASH SHA256=d6c65aca6b1ed68e7a182f4757257b107ae403032760ed6ef121c9d55e81757d
)

set(JSON_BuildTests OFF CACHE INTERNAL "")
set(JSON_Install OFF CACHE INTERNAL "")

FetchContent_MakeAvailable(nlohmann_json)

# ---- Your executable ----
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp"
)

# src/Platform/ is unused SFML-template boilerplate -- nothing in the project includes
# Platform.hpp. GLOB_RECURSE was pulling it into the build anyway, where LinuxPlatform.cpp
# calls XInitThreads() and broke the link (X11 is never linked here). Excluded rather than
# deleted, in case it's wanted later; to re-enable, drop this filter and link X11 on Linux.
list(FILTER SOURCES EXCLUDE REGEX "/src/Platform/")

message(STATUS "Found source files: ${SOURCES}")


add_executable(starmap3 ${SOURCES})

target_link_libraries(starmap3 PRIVATE sfml-graphics nlohmann_json::nlohmann_json)

target_compile_features(starmap3 PRIVATE cxx_std_17)

# ---- Warnings ----
# Deliberately NOT -Werror: this turns the compiler into a reviewer without
# blocking the build while the existing warnings get worked through.
if (MSVC)
    target_compile_options(starmap3 PRIVATE /W4)
else()
    target_compile_options(starmap3 PRIVATE -Wall -Wextra)
endif()

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

# ---- Runtime content ----
# The program opens ./content/hygdata_v40.csv, ./content/config.json and
# ./content/Frontier.ttf relative to its working directory, so the data has to sit
# beside the executable to be runnable. Copying it here rather than in the CI
# packaging step means every build gets it -- CI, local, any generator -- and the
# packaged artifact is runnable without a separate copy step remembering to do it.
if (CMAKE_VERSION VERSION_GREATER_EQUAL 3.26)
    set(STARMAP_COPY_DIR_COMMAND copy_directory_if_different)
else()
    set(STARMAP_COPY_DIR_COMMAND copy_directory)
endif()

add_custom_command(
    TARGET starmap3
    POST_BUILD
    COMMENT "Copy ./content next to the executable"
    COMMAND ${CMAKE_COMMAND} -E ${STARMAP_COPY_DIR_COMMAND}
        ${CMAKE_CURRENT_LIST_DIR}/content
        $<TARGET_FILE_DIR:starmap3>/content
)

# ---- Install rule (optional, but fine) ----
install(TARGETS starmap3)
install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/content/ DESTINATION bin/content)
