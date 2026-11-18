cmake_minimum_required(VERSION 3.10)
project(SmartPlaylistSynthesizer VERSION 1.0 LANGUAGES CXX)

# C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include headers
include_directories(include)

# ── Shared Core Domain Library ─────────────────────────────────────────

set(CORE_SOURCES
    src/Song.cpp
    src/MusicLibrary.cpp
    src/Playlist.cpp
    src/PlaylistManager.cpp
    src/MusicPlayer.cpp
    src/PlaylistStorage.cpp
)

# ── 1. Original Console Application ────────────────────────────────────

add_executable(SmartPlaylistSynthesizer
    src/main.cpp
    src/Application.cpp
    ${CORE_SOURCES}
)

# ── 2. Web Application Server ──────────────────────────────────────────

add_executable(SmartPlaylistSynthesizerWeb
    src/web_main.cpp
    src/WebServer.cpp
    ${CORE_SOURCES}
)

if(WIN32)
    target_link_libraries(SmartPlaylistSynthesizerWeb PRIVATE ws2_32 crypt32)
endif()

# ── 3. Test Executables ────────────────────────────────────────────────

add_executable(playlist_tests
    tests/playlist_tests.cpp
    src/Song.cpp
    src/MusicLibrary.cpp
    src/Playlist.cpp
    src/PlaylistManager.cpp
    src/MusicPlayer.cpp
)

add_executable(storage_tests
    tests/storage_tests.cpp
    src/Song.cpp
    src/Playlist.cpp
    src/PlaylistStorage.cpp
)

add_executable(api_tests
    tests/api_tests.cpp
    src/WebServer.cpp
    ${CORE_SOURCES}
)

if(WIN32)
    target_link_libraries(api_tests PRIVATE ws2_32 crypt32)
endif()

# ── Compiler Options & Warnings ────────────────────────────────────────

if(MSVC)
    target_compile_options(SmartPlaylistSynthesizer PRIVATE /W4)
    target_compile_options(SmartPlaylistSynthesizerWeb PRIVATE /W4)
    target_compile_options(playlist_tests PRIVATE /W4)
    target_compile_options(storage_tests PRIVATE /W4)
    target_compile_options(api_tests PRIVATE /W4)
else()
    target_compile_options(SmartPlaylistSynthesizer PRIVATE -Wall -Wextra)
    target_compile_options(SmartPlaylistSynthesizerWeb PRIVATE -Wall -Wextra)
    target_compile_options(playlist_tests PRIVATE -Wall -Wextra)
    target_compile_options(storage_tests PRIVATE -Wall -Wextra)
    target_compile_options(api_tests PRIVATE -Wall -Wextra)
endif()
