#include "MusicLibrary.h"
#include <iostream>
#include <iomanip>
#include <cctype>

// ══════════════════════════════════════════════════════════════════════
// Constructor
// ══════════════════════════════════════════════════════════════════════

MusicLibrary::MusicLibrary() {}

// ══════════════════════════════════════════════════════════════════════
// Core operations
// ══════════════════════════════════════════════════════════════════════

bool MusicLibrary::addSong(const Song& song) {
    // Step 1: Check if ID already exists in the set — O(1) average
    if (songIds.count(song.getId()) > 0) {
        return false;   // Duplicate detected
    }
    // Step 2: Register the ID in the set
    songIds.insert(song.getId());
    // Step 3: Append the song to the vector
    songs.push_back(song);
    return true;
}

bool MusicLibrary::updateSong(const Song& song) {
    auto it = std::find_if(songs.begin(), songs.end(),
                           [&song](const Song& s) { return s.getId() == song.getId(); });
    if (it != songs.end()) {
        *it = song;
        return true;
    }
    return false;
}

bool MusicLibrary::removeSong(int id) {
    // Step 1: Verify the song exists
    if (songIds.count(id) == 0) {
        return false;
    }
    // Step 2: Remove from the vector
    //         Uses erase-remove idiom with a lambda predicate
    songs.erase(
        std::remove_if(songs.begin(), songs.end(),
                       [id](const Song& s) { return s.getId() == id; }),
        songs.end()
    );
    // Step 3: Remove from the ID set
    songIds.erase(id);
    return true;
}

void MusicLibrary::clear() {
    songs.clear();
    songIds.clear();
}

bool MusicLibrary::containsSong(int id) const {
    return songIds.count(id) > 0;
}

int MusicLibrary::getSongCount() const {
    return static_cast<int>(songs.size());
}

// ══════════════════════════════════════════════════════════════════════
// Search operations
// ══════════════════════════════════════════════════════════════════════

const Song* MusicLibrary::findById(int id) const {
    auto it = std::find_if(songs.begin(), songs.end(),
                           [id](const Song& s) { return s.getId() == id; });
    if (it != songs.end()) {
        return &(*it);
    }
    return nullptr;
}

std::vector<Song> MusicLibrary::searchByTitle(const std::string& query) const {
    std::vector<Song> results;
    for (const auto& song : songs) {
        if (containsIgnoreCase(song.getTitle(), query)) {
            results.push_back(song);
        }
    }
    return results;
}

std::vector<Song> MusicLibrary::searchByArtist(const std::string& query) const {
    std::vector<Song> results;
    for (const auto& song : songs) {
        if (containsIgnoreCase(song.getArtist(), query)) {
            results.push_back(song);
        }
    }
    return results;
}

std::vector<Song> MusicLibrary::searchByGenre(const std::string& query) const {
    std::vector<Song> results;
    for (const auto& song : songs) {
        if (containsIgnoreCase(song.getGenre(), query)) {
            results.push_back(song);
        }
    }
    return results;
}

// ══════════════════════════════════════════════════════════════════════
// Display
// ══════════════════════════════════════════════════════════════════════

void MusicLibrary::displayHeader() const {
    std::cout << "\n  " << std::left
              << std::setw(5)  << "ID"
              << std::setw(30) << "Title"
              << std::setw(22) << "Artist"
              << std::setw(14) << "Genre"
              << std::right << std::setw(6) << "Time"
              << "\n  " << std::string(77, '-') << std::endl;
}

void MusicLibrary::displayAll() const {
    if (songs.empty()) {
        std::cout << "\n  [!] The music library is empty.\n";
        return;
    }
    std::cout << "\n  ============= MUSIC LIBRARY (" << songs.size()
              << " tracks) =============\n";
    displayHeader();
    for (const auto& song : songs) {
        song.display();
    }
    std::cout << "  " << std::string(77, '=') << "\n";
}

const std::vector<Song>& MusicLibrary::getAllSongs() const {
    return songs;
}

// ══════════════════════════════════════════════════════════════════════
// Sample data
// ══════════════════════════════════════════════════════════════════════

void MusicLibrary::loadSampleData() {
    // 10 sample tracks — fictional titles for demonstration
    addSong(Song(101, "Midnight Drive",       "Echo Nova",        "Electronic", 224));
    addSong(Song(102, "Fading Echoes",        "Luna Vox",         "Indie Pop",  198));
    addSong(Song(103, "Thunder Road",         "Steel Horizon",    "Rock",       263));
    addSong(Song(104, "City Lights",          "Neon Pulse",       "Synthwave",  210));
    addSong(Song(105, "Starfall",             "Aria Sky",         "Pop",        185));
    addSong(Song(106, "Rust and Roses",       "The Iron Blooms",  "Alternative",241));
    addSong(Song(107, "Digital Rain",         "CyberDrift",       "Electronic", 195));
    addSong(Song(108, "Broken Compass",       "Wanderer",         "Folk",       217));
    addSong(Song(109, "Neon Shadows",         "Glitch Theory",    "Synthwave",  232));
    addSong(Song(110, "Ocean Lullaby",        "Tidal Waves",      "Ambient",    278));
}

// ══════════════════════════════════════════════════════════════════════
// Internal helpers
// ══════════════════════════════════════════════════════════════════════

bool MusicLibrary::containsIgnoreCase(const std::string& haystack,
                                       const std::string& needle) {
    if (needle.empty()) return true;
    // Convert both strings to lowercase, then check for substring
    std::string lowerHaystack = haystack;
    std::string lowerNeedle   = needle;
    std::transform(lowerHaystack.begin(), lowerHaystack.end(),
                   lowerHaystack.begin(), ::tolower);
    std::transform(lowerNeedle.begin(), lowerNeedle.end(),
                   lowerNeedle.begin(), ::tolower);
    return lowerHaystack.find(lowerNeedle) != std::string::npos;
}
