/**
 * @file playlist_tests.cpp
 * @brief Automated tests for Song, MusicLibrary, Playlist, PlaylistManager,
 *        and MusicPlayer classes.
 *
 * Uses a lightweight assertion macro instead of an external test framework.
 * Each test prints PASS or FAIL with a description.
 *
 * Compile:
 *   g++ -std=c++17 -Wall -Wextra -Iinclude tests/playlist_tests.cpp \
 *       src/Song.cpp src/MusicLibrary.cpp src/Playlist.cpp \
 *       src/PlaylistManager.cpp src/MusicPlayer.cpp -o playlist_tests.exe
 *
 * Run:
 *   playlist_tests.exe
 */

#include "Song.h"
#include "MusicLibrary.h"
#include "Playlist.h"
#include "PlaylistManager.h"
#include "MusicPlayer.h"

#include <iostream>
#include <cassert>
#include <string>
#include <vector>
#include <unordered_set>

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
// Song Tests
// ══════════════════════════════════════════════════════════════════════

void testSongCreation() {
    std::cout << "\n  --- Song Tests ---\n";

    Song s(1, "Test Song", "Test Artist", "Pop", 200);
    TEST("Song creation — ID",       s.getId() == 1);
    TEST("Song creation — title",    s.getTitle() == "Test Song");
    TEST("Song creation — artist",   s.getArtist() == "Test Artist");
    TEST("Song creation — genre",    s.getGenre() == "Pop");
    TEST("Song creation — duration", s.getDurationSeconds() == 200);
    TEST("Song formatted duration",  s.getFormattedDuration() == "3:20");

    // Test equality
    Song s2(1, "Different", "Different", "Rock", 100);
    TEST("Song equality (same ID)",   s == s2);
    Song s3(2, "Test Song", "Test Artist", "Pop", 200);
    TEST("Song inequality (diff ID)", s != s3);
}

void testSongValidation() {
    bool caught = false;

    // Negative ID
    try { Song(0, "T", "A", "G", 100); } catch (const std::invalid_argument&) { caught = true; }
    TEST("Song rejects ID <= 0", caught);

    // Empty title
    caught = false;
    try { Song(1, "", "A", "G", 100); } catch (const std::invalid_argument&) { caught = true; }
    TEST("Song rejects empty title", caught);

    // Empty artist
    caught = false;
    try { Song(1, "T", "", "G", 100); } catch (const std::invalid_argument&) { caught = true; }
    TEST("Song rejects empty artist", caught);

    // Empty genre
    caught = false;
    try { Song(1, "T", "A", "", 100); } catch (const std::invalid_argument&) { caught = true; }
    TEST("Song rejects empty genre", caught);

    // Negative duration
    caught = false;
    try { Song(1, "T", "A", "G", -5); } catch (const std::invalid_argument&) { caught = true; }
    TEST("Song rejects negative duration", caught);
}

// ══════════════════════════════════════════════════════════════════════
// MusicLibrary Tests
// ══════════════════════════════════════════════════════════════════════

void testMusicLibrary() {
    std::cout << "\n  --- MusicLibrary Tests ---\n";

    MusicLibrary lib;
    TEST("Library starts empty", lib.getSongCount() == 0);

    Song s1(1, "Alpha", "Artist A", "Rock", 180);
    Song s2(2, "Beta",  "Artist B", "Pop",  200);

    TEST("Add valid song",         lib.addSong(s1) == true);
    TEST("Library count after add", lib.getSongCount() == 1);
    TEST("Add second song",       lib.addSong(s2) == true);
    TEST("Library count = 2",      lib.getSongCount() == 2);

    // Duplicate rejection
    Song dup(1, "Duplicate", "Dup", "Dup", 100);
    TEST("Reject duplicate ID",   lib.addSong(dup) == false);
    TEST("Count unchanged",       lib.getSongCount() == 2);

    // Search by ID
    const Song* found = lib.findById(1);
    TEST("Find by ID — found",          found != nullptr);
    TEST("Find by ID — correct title",  found != nullptr && found->getTitle() == "Alpha");
    TEST("Find by ID — not found",      lib.findById(999) == nullptr);

    // Search by title
    auto results = lib.searchByTitle("alp");
    TEST("Search by title (case-insensitive)", results.size() == 1);

    // Search by artist
    results = lib.searchByArtist("artist");
    TEST("Search by artist — both match", results.size() == 2);

    // Remove
    TEST("Remove existing song",     lib.removeSong(1) == true);
    TEST("Count after removal",      lib.getSongCount() == 1);
    TEST("Remove non-existent song", lib.removeSong(999) == false);

    // Contains
    TEST("Contains after removal",   lib.containsSong(1) == false);
    TEST("Contains existing",        lib.containsSong(2) == true);
}

void testLibrarySampleData() {
    std::cout << "\n  --- Library Sample Data Tests ---\n";

    MusicLibrary lib;
    lib.loadSampleData();
    TEST("Sample data loaded", lib.getSongCount() == 10);
    TEST("Sample ID 101 exists", lib.containsSong(101) == true);
    TEST("Sample ID 110 exists", lib.containsSong(110) == true);

    // No duplicate loading
    int before = lib.getSongCount();
    lib.loadSampleData();  // Should not add duplicates
    TEST("Duplicate sample data rejected", lib.getSongCount() == before);
}

// ══════════════════════════════════════════════════════════════════════
// Playlist Tests
// ══════════════════════════════════════════════════════════════════════

void testPlaylistCore() {
    std::cout << "\n  --- Playlist Core Tests ---\n";

    Playlist pl("Test Playlist");
    TEST("Playlist name",    pl.getName() == "Test Playlist");
    TEST("Playlist empty",   pl.isEmpty() == true);
    TEST("Playlist count 0", pl.getTrackCount() == 0);

    Song s1(1, "Song A", "Art A", "Rock", 180);
    Song s2(2, "Song B", "Art B", "Pop",  200);
    Song s3(3, "Song C", "Art C", "Jazz", 150);

    TEST("Add first track",    pl.addSong(s1) == true);
    TEST("Add second track",   pl.addSong(s2) == true);
    TEST("Track count = 2",    pl.getTrackCount() == 2);
    TEST("Not empty",          pl.isEmpty() == false);

    // DUPLICATE PREVENTION (official requirement)
    TEST("Reject duplicate ID", pl.addSong(s1) == false);
    TEST("Count unchanged",     pl.getTrackCount() == 2);

    // Contains
    TEST("Contains ID 1", pl.containsSong(1) == true);
    TEST("Contains ID 2", pl.containsSong(2) == true);
    TEST("Does not contain ID 99", pl.containsSong(99) == false);

    // Total duration
    TEST("Total duration", pl.getTotalDuration() == 380);

    // Add third
    pl.addSong(s3);

    // Remove
    TEST("Remove existing",     pl.removeSong(2) == true);
    TEST("Count after remove",  pl.getTrackCount() == 2);
    TEST("Removed ID gone",     pl.containsSong(2) == false);
    TEST("Remove non-existent", pl.removeSong(999) == false);

    // getTrackAt
    bool outOfRange = false;
    try { pl.getTrackAt(999); } catch (const std::out_of_range&) { outOfRange = true; }
    TEST("getTrackAt out of range", outOfRange);

    // Rename
    pl.setName("Renamed");
    TEST("Rename playlist", pl.getName() == "Renamed");
}

void testPlaylistDuplicateAcross() {
    std::cout << "\n  --- Cross-Playlist Duplicate Tests ---\n";

    Playlist pl1("Playlist 1");
    Playlist pl2("Playlist 2");
    Song s(1, "Shared Song", "Artist", "Genre", 100);

    TEST("Song in playlist 1",  pl1.addSong(s) == true);
    TEST("Same song in playlist 2", pl2.addSong(s) == true);
    TEST("Both contain ID 1",
         pl1.containsSong(1) && pl2.containsSong(1));
}

// ══════════════════════════════════════════════════════════════════════
// Shuffle Tests
// ══════════════════════════════════════════════════════════════════════

void testPlaylistShuffle() {
    std::cout << "\n  --- Shuffle Tests ---\n";

    // Empty playlist
    Playlist empty("Empty");
    empty.shuffle();  // Should not crash
    TEST("Shuffle empty playlist — no crash", true);

    // Single-song playlist
    Playlist single("Single");
    single.addSong(Song(1, "Solo", "Art", "Pop", 100));
    single.shuffle();
    TEST("Shuffle single song — count preserved", single.getTrackCount() == 1);

    // Multi-song playlist
    Playlist pl("Shuffle Test");
    for (int i = 1; i <= 10; ++i) {
        pl.addSong(Song(i, "Song " + std::to_string(i),
                        "Artist", "Pop", 100 + i));
    }

    // Record original IDs
    std::unordered_set<int> originalIds;
    for (const auto& t : pl.getTracks()) {
        originalIds.insert(t.getId());
    }

    // Shuffle with a fixed seed for reproducibility
    pl.shuffle(42u);

    // Verify count preserved
    TEST("Shuffle preserves count", pl.getTrackCount() == 10);

    // Verify all IDs preserved
    std::unordered_set<int> shuffledIds;
    for (const auto& t : pl.getTracks()) {
        shuffledIds.insert(t.getId());
    }
    TEST("Shuffle preserves all IDs", originalIds == shuffledIds);

    // Verify no duplicates after shuffle
    TEST("No duplicates after shuffle",
         static_cast<int>(shuffledIds.size()) == pl.getTrackCount());

    // Repeated shuffles preserve membership
    for (int round = 0; round < 5; ++round) {
        pl.shuffle();
    }
    std::unordered_set<int> afterRepeated;
    for (const auto& t : pl.getTracks()) {
        afterRepeated.insert(t.getId());
    }
    TEST("Repeated shuffles preserve IDs", originalIds == afterRepeated);
    TEST("Count after repeated shuffles",  pl.getTrackCount() == 10);
}

// ══════════════════════════════════════════════════════════════════════
// PlaylistManager Tests
// ══════════════════════════════════════════════════════════════════════

void testPlaylistManager() {
    std::cout << "\n  --- PlaylistManager Tests ---\n";

    PlaylistManager mgr;
    TEST("Manager starts empty", mgr.getPlaylistCount() == 0);

    TEST("Create playlist",      mgr.createPlaylist("My Mix") == true);
    TEST("Count = 1",            mgr.getPlaylistCount() == 1);
    TEST("Reject duplicate name", mgr.createPlaylist("My Mix") == false);
    TEST("Create another",       mgr.createPlaylist("Road Trip") == true);
    TEST("Count = 2",            mgr.getPlaylistCount() == 2);

    // Reject empty name
    TEST("Reject empty name",    mgr.createPlaylist("") == false);

    // Find by name
    TEST("Find 'My Mix'",       mgr.findByName("My Mix") == 0);
    TEST("Find 'Road Trip'",    mgr.findByName("Road Trip") == 1);
    TEST("Find non-existent",   mgr.findByName("None") == -1);

    // Rename
    TEST("Rename success",       mgr.renamePlaylist(0, "Evening Chill") == true);
    TEST("Rename to taken name", mgr.renamePlaylist(0, "Road Trip") == false);
    TEST("Verify rename",        mgr.getPlaylistConst(0).getName() == "Evening Chill");

    // Delete
    TEST("Delete playlist",      mgr.deletePlaylist(1) == true);
    TEST("Count after delete",   mgr.getPlaylistCount() == 1);
    TEST("Delete invalid index", mgr.deletePlaylist(99) == false);
}

// ══════════════════════════════════════════════════════════════════════
// MusicPlayer Tests
// ══════════════════════════════════════════════════════════════════════

void testMusicPlayer() {
    std::cout << "\n  --- MusicPlayer Tests ---\n";

    MusicPlayer player;
    TEST("Player starts stopped", player.getState() == MusicPlayer::State::STOPPED);
    TEST("No playlist loaded",    player.hasPlaylist() == false);
    TEST("Current song is null",  player.getCurrentSong() == nullptr);

    // Load a playlist
    Playlist pl("Player Test");
    pl.addSong(Song(1, "Track 1", "Artist", "Pop", 100));
    pl.addSong(Song(2, "Track 2", "Artist", "Pop", 120));
    pl.addSong(Song(3, "Track 3", "Artist", "Pop", 140));

    player.loadPlaylist(&pl);
    TEST("Playlist loaded",      player.hasPlaylist() == true);
    TEST("State after load",     player.getState() == MusicPlayer::State::STOPPED);

    // Play
    player.play();
    TEST("State after play",     player.getState() == MusicPlayer::State::PLAYING);
    TEST("Current track index",  player.getCurrentTrackIndex() == 0);
    const Song* cur = player.getCurrentSong();
    TEST("Current song not null", cur != nullptr);
    TEST("Current song ID = 1",   cur != nullptr && cur->getId() == 1);

    // Pause
    player.pause();
    TEST("State after pause",    player.getState() == MusicPlayer::State::PAUSED);

    // Resume
    player.resume();
    TEST("State after resume",   player.getState() == MusicPlayer::State::PLAYING);

    // Next
    player.next();
    TEST("After next — index 1", player.getCurrentTrackIndex() == 1);
    cur = player.getCurrentSong();
    TEST("After next — ID 2",    cur != nullptr && cur->getId() == 2);

    // Next again
    player.next();
    TEST("After next — index 2", player.getCurrentTrackIndex() == 2);

    // Next at end — should not advance
    player.next();
    TEST("Next at end — stays",  player.getCurrentTrackIndex() == 2);

    // Previous
    player.previous();
    TEST("After previous — index 1", player.getCurrentTrackIndex() == 1);

    // Previous back to start
    player.previous();
    TEST("After previous — index 0", player.getCurrentTrackIndex() == 0);

    // Previous at start — should not go below 0
    player.previous();
    TEST("Previous at start — stays", player.getCurrentTrackIndex() == 0);

    // Test empty playlist
    Playlist emptyPl("Empty");
    MusicPlayer emptyPlayer;
    emptyPlayer.loadPlaylist(&emptyPl);
    TEST("Empty playlist — hasPlaylist false", emptyPlayer.hasPlaylist() == false);
}

// ══════════════════════════════════════════════════════════════════════
// Search & Sort Tests
// ══════════════════════════════════════════════════════════════════════

void testSearchAndSort() {
    std::cout << "\n  --- Search & Sort Tests ---\n";

    Playlist pl("Search Test");
    pl.addSong(Song(1, "Zebra Song",   "Alpha Artist", "Rock",  300));
    pl.addSong(Song(2, "Apple Song",   "Beta Artist",  "Pop",   200));
    pl.addSong(Song(3, "Mango Track",  "Gamma Artist", "Jazz",  250));

    // Search by title
    auto r = pl.searchByTitle("song");
    TEST("Search title 'song' — 2 results", r.size() == 2);

    // Search by artist
    r = pl.searchByArtist("beta");
    TEST("Search artist 'beta' — 1 result", r.size() == 1);

    // Search by genre
    r = pl.searchByGenre("ROCK");
    TEST("Search genre 'ROCK' (case-insensitive)", r.size() == 1);

    // Find by ID
    const Song* s = pl.findById(2);
    TEST("Find by ID 2", s != nullptr && s->getTitle() == "Apple Song");
    TEST("Find by ID 99 — null", pl.findById(99) == nullptr);

    // Sort by title
    pl.sortByTitle();
    TEST("Sort by title — first",  pl.getTrackAt(0).getTitle() == "Apple Song");
    TEST("Sort by title — last",   pl.getTrackAt(2).getTitle() == "Zebra Song");

    // Sort by duration
    pl.sortByDuration();
    TEST("Sort by duration — shortest first", pl.getTrackAt(0).getDurationSeconds() == 200);
    TEST("Sort by duration — longest last",   pl.getTrackAt(2).getDurationSeconds() == 300);

    // Sort by artist
    pl.sortByArtist();
    TEST("Sort by artist — first", pl.getTrackAt(0).getArtist() == "Alpha Artist");
}

// ══════════════════════════════════════════════════════════════════════
// Main test runner
// ══════════════════════════════════════════════════════════════════════

int main() {
    std::cout << "\n  ====================================================\n";
    std::cout << "  Smart Playlist Synthesizer — Playlist & Core Tests\n";
    std::cout << "  ====================================================\n";

    testSongCreation();
    testSongValidation();
    testMusicLibrary();
    testLibrarySampleData();
    testPlaylistCore();
    testPlaylistDuplicateAcross();
    testPlaylistShuffle();
    testPlaylistManager();
    testMusicPlayer();
    testSearchAndSort();

    std::cout << "\n  ====================================================\n";
    std::cout << "  Results: " << passedTests << " passed, "
              << failedTests << " failed, " << totalTests << " total\n";
    std::cout << "  ====================================================\n\n";

    return failedTests > 0 ? 1 : 0;
}
