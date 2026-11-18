/**
 * @file storage_tests.cpp
 * @brief Automated tests for PlaylistStorage — save, load, error handling.
 *
 * Compile:
 *   g++ -std=c++17 -Wall -Wextra -Iinclude tests/storage_tests.cpp \
 *       src/Song.cpp src/Playlist.cpp src/PlaylistStorage.cpp -o storage_tests.exe
 *
 * Run:
 *   storage_tests.exe
 */

#include "Song.h"
#include "Playlist.h"
#include "PlaylistStorage.h"

#include <iostream>
#include <fstream>
#include <cstdio>
#include <string>

// ── Lightweight test harness ──────────────────────────────────────────

static int totalTests  = 0;
static int passedTests = 0;
static int failedTests = 0;

#define TEST(name, expr)                                            \
    do {                                                            \
        ++totalTests;                                               \
        if (expr) {                                                 \
            ++passedTests;                                          \
            std::cout << "  [PASS] " << name << "\n";              \
        } else {                                                    \
            ++failedTests;                                          \
            std::cout << "  [FAIL] " << name                       \
                      << " (line " << __LINE__ << ")\n";            \
        }                                                           \
    } while (0)

// ══════════════════════════════════════════════════════════════════════
// Helper: create a test playlist
// ══════════════════════════════════════════════════════════════════════

Playlist createTestPlaylist() {
    Playlist pl("Test Playlist");
    pl.addSong(Song(1, "Alpha Song",  "Alpha Artist",  "Rock",      180));
    pl.addSong(Song(2, "Beta Track",  "Beta Singer",   "Pop",       240));
    pl.addSong(Song(3, "Gamma Tune",  "Gamma Band",    "Jazz",      195));
    return pl;
}

// ══════════════════════════════════════════════════════════════════════
// Save & Load Tests
// ══════════════════════════════════════════════════════════════════════

void testSaveAndLoad() {
    std::cout << "\n  --- Save & Load Tests ---\n";

    PlaylistStorage storage;
    Playlist original = createTestPlaylist();
    std::string filepath = "test_save_load.csv";

    // Save
    bool saved = storage.savePlaylist(original, filepath);
    TEST("Save succeeds", saved);

    // Verify file exists
    std::ifstream check(filepath);
    TEST("File created", check.good());
    check.close();

    // Load
    Playlist loaded;
    bool loadResult = storage.loadPlaylist(filepath, loaded);
    TEST("Load succeeds", loadResult);
    TEST("Loaded name matches",  loaded.getName() == "Test Playlist");
    TEST("Loaded count matches", loaded.getTrackCount() == 3);

    // Verify track data
    const Song* s1 = loaded.findById(1);
    TEST("Track 1 loaded",           s1 != nullptr);
    TEST("Track 1 title preserved",  s1 != nullptr && s1->getTitle() == "Alpha Song");
    TEST("Track 1 artist preserved", s1 != nullptr && s1->getArtist() == "Alpha Artist");
    TEST("Track 1 genre preserved",  s1 != nullptr && s1->getGenre() == "Rock");
    TEST("Track 1 duration preserved", s1 != nullptr && s1->getDurationSeconds() == 180);

    const Song* s2 = loaded.findById(2);
    TEST("Track 2 loaded",  s2 != nullptr);
    TEST("Track 2 title",   s2 != nullptr && s2->getTitle() == "Beta Track");

    const Song* s3 = loaded.findById(3);
    TEST("Track 3 loaded",  s3 != nullptr);

    // Verify duplicate-prevention set is rebuilt
    TEST("Duplicate set rebuilt — contains 1", loaded.containsSong(1));
    TEST("Duplicate set rebuilt — contains 2", loaded.containsSong(2));
    TEST("Duplicate set rebuilt — contains 3", loaded.containsSong(3));

    // Try adding a duplicate after loading
    Song dup(1, "Duplicate", "Dup", "Dup", 100);
    TEST("Reject duplicate after load", loaded.addSong(dup) == false);

    // Clean up test file
    std::remove(filepath.c_str());
}

// ══════════════════════════════════════════════════════════════════════
// Missing File Test
// ══════════════════════════════════════════════════════════════════════

void testMissingFile() {
    std::cout << "\n  --- Missing File Tests ---\n";

    PlaylistStorage storage;
    Playlist pl;
    bool result = storage.loadPlaylist("nonexistent_file_xyz.csv", pl);
    TEST("Load missing file fails gracefully", result == false);
    TEST("Playlist unchanged after failed load", pl.isEmpty());
}

// ══════════════════════════════════════════════════════════════════════
// Malformed Data Test
// ══════════════════════════════════════════════════════════════════════

void testMalformedData() {
    std::cout << "\n  --- Malformed Data Tests ---\n";

    // Create a file with mixed valid and invalid records
    std::string filepath = "test_malformed.csv";
    {
        std::ofstream f(filepath);
        f << "PLAYLIST:Malformed Test\n";
        f << "1,Valid Song,Valid Artist,Pop,200\n";     // Valid
        f << "not_a_number,Bad,Bad,Bad,100\n";          // Invalid ID
        f << "2,Good Song,Good Artist,Rock,180\n";     // Valid
        f << "3,,Missing Artist,Jazz,150\n";            // Invalid: empty title
        f << "4,No Duration,Artist,Pop,\n";             // Invalid: missing duration
        f << "5,Last Song,Last Artist,Blues,220\n";     // Valid
        f.close();
    }

    PlaylistStorage storage;
    Playlist pl;
    bool result = storage.loadPlaylist(filepath, pl);
    TEST("Load with malformed data succeeds", result == true);
    TEST("Valid records loaded (3 of 6)", pl.getTrackCount() == 3);
    TEST("Track 1 loaded", pl.containsSong(1));
    TEST("Track 2 loaded", pl.containsSong(2));
    TEST("Track 5 loaded", pl.containsSong(5));

    // Clean up
    std::remove(filepath.c_str());
}

// ══════════════════════════════════════════════════════════════════════
// Empty Playlist Save/Load Test
// ══════════════════════════════════════════════════════════════════════

void testEmptyPlaylist() {
    std::cout << "\n  --- Empty Playlist Save/Load Tests ---\n";

    PlaylistStorage storage;
    Playlist empty("Empty Playlist");
    std::string filepath = "test_empty.csv";

    bool saved = storage.savePlaylist(empty, filepath);
    TEST("Save empty playlist", saved);

    Playlist loaded;
    bool loadResult = storage.loadPlaylist(filepath, loaded);
    TEST("Load empty playlist", loadResult);
    TEST("Loaded is empty",     loaded.isEmpty());
    TEST("Loaded name correct", loaded.getName() == "Empty Playlist");

    std::remove(filepath.c_str());
}

// ══════════════════════════════════════════════════════════════════════
// Filename Generation Test
// ══════════════════════════════════════════════════════════════════════

void testFilenameGeneration() {
    std::cout << "\n  --- Filename Generation Tests ---\n";

    TEST("Simple name", PlaylistStorage::generateFilename("My Playlist") == "my_playlist.csv");
    TEST("Special chars", PlaylistStorage::generateFilename("Rock & Roll!") == "rock___roll_.csv");
}

// ══════════════════════════════════════════════════════════════════════
// Main
// ══════════════════════════════════════════════════════════════════════

int main() {
    std::cout << "\n  ====================================================\n";
    std::cout << "  Smart Playlist Synthesizer — Storage Tests\n";
    std::cout << "  ====================================================\n";

    testSaveAndLoad();
    testMissingFile();
    testMalformedData();
    testEmptyPlaylist();
    testFilenameGeneration();

    std::cout << "\n  ====================================================\n";
    std::cout << "  Results: " << passedTests << " passed, "
              << failedTests << " failed, " << totalTests << " total\n";
    std::cout << "  ====================================================\n\n";

    return failedTests > 0 ? 1 : 0;
}
