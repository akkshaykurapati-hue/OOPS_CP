#include "Playlist.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cctype>
#include <stdexcept>
#include <chrono>

// ══════════════════════════════════════════════════════════════════════
// Constructors
// ══════════════════════════════════════════════════════════════════════

Playlist::Playlist() : name("Untitled Playlist") {}

Playlist::Playlist(const std::string& name) : name(name) {}

// ══════════════════════════════════════════════════════════════════════
// Getters / Setters
// ══════════════════════════════════════════════════════════════════════

std::string Playlist::getName() const { return name; }
int  Playlist::getTrackCount() const { return static_cast<int>(tracks.size()); }
bool Playlist::isEmpty() const { return tracks.empty(); }
void Playlist::setName(const std::string& newName) { name = newName; }

// ══════════════════════════════════════════════════════════════════════
// Core operations — OFFICIAL REQUIREMENT: duplicate prevention
// ══════════════════════════════════════════════════════════════════════

bool Playlist::addSong(const Song& song) {
    /*
     * Step 1: Check the unordered_set for the song's ID.
     *         Average-case complexity: O(1).
     *         Worst-case (hash collisions): O(n).
     *
     * Step 2: If the ID is already present, reject the insertion.
     *
     * Step 3: Otherwise, insert the ID into the set AND append
     *         the Song to the vector. Both structures must be
     *         updated atomically to stay consistent.
     */
    if (trackIds.count(song.getId()) > 0) {
        return false;   // Duplicate detected — reject
    }
    trackIds.insert(song.getId());
    tracks.push_back(song);
    return true;
}

bool Playlist::updateSong(const Song& song) {
    if (trackIds.count(song.getId()) == 0) {
        return false;
    }
    auto it = std::find_if(tracks.begin(), tracks.end(),
                           [&song](const Song& s) { return s.getId() == song.getId(); });
    if (it != tracks.end()) {
        *it = song;
        return true;
    }
    return false;
}

bool Playlist::removeSong(int id) {
    /*
     * Remove from BOTH data structures to maintain consistency.
     * Step 1: Verify the ID exists in the set.
     * Step 2: Erase from the vector using the erase-remove idiom.
     * Step 3: Erase from the set.
     */
    if (trackIds.count(id) == 0) {
        return false;   // Not found
    }
    tracks.erase(
        std::remove_if(tracks.begin(), tracks.end(),
                       [id](const Song& s) { return s.getId() == id; }),
        tracks.end()
    );
    trackIds.erase(id);
    return true;
}

bool Playlist::containsSong(int id) const {
    return trackIds.count(id) > 0;
}

int Playlist::getTotalDuration() const {
    int total = 0;
    for (const auto& t : tracks) {
        total += t.getDurationSeconds();
    }
    return total;
}

std::string Playlist::getFormattedTotalDuration() const {
    int total = getTotalDuration();
    int hours   = total / 3600;
    int minutes = (total % 3600) / 60;
    int seconds = total % 60;
    std::ostringstream oss;
    if (hours > 0) {
        oss << hours << "h ";
    }
    oss << minutes << "m " << seconds << "s";
    return oss.str();
}

// ══════════════════════════════════════════════════════════════════════
// Shuffling — OFFICIAL REQUIREMENT
// ══════════════════════════════════════════════════════════════════════

void Playlist::shuffle() {
    /*
     * Uses a time-based seed for non-reproducible shuffling during
     * normal program usage.
     *
     * std::shuffle performs a Fisher–Yates shuffle on the vector,
     * producing a uniformly random permutation. Complexity: O(n).
     *
     * The unordered_set does NOT change because shuffling only
     * rearranges the vector order; it does not add or remove songs.
     */
    if (tracks.size() <= 1) return;   // Nothing to shuffle

    unsigned int seed = static_cast<unsigned int>(
        std::chrono::system_clock::now().time_since_epoch().count()
    );
    std::mt19937 rng(seed);
    std::shuffle(tracks.begin(), tracks.end(), rng);
}

void Playlist::shuffle(unsigned int seed) {
    // Reproducible shuffle for testing
    if (tracks.size() <= 1) return;
    std::mt19937 rng(seed);
    std::shuffle(tracks.begin(), tracks.end(), rng);
}

// ══════════════════════════════════════════════════════════════════════
// Search
// ══════════════════════════════════════════════════════════════════════

const Song* Playlist::findById(int id) const {
    auto it = std::find_if(tracks.begin(), tracks.end(),
                           [id](const Song& s) { return s.getId() == id; });
    if (it != tracks.end()) return &(*it);
    return nullptr;
}

std::vector<Song> Playlist::searchByTitle(const std::string& query) const {
    std::vector<Song> results;
    for (const auto& t : tracks) {
        if (containsIgnoreCase(t.getTitle(), query))
            results.push_back(t);
    }
    return results;
}

std::vector<Song> Playlist::searchByArtist(const std::string& query) const {
    std::vector<Song> results;
    for (const auto& t : tracks) {
        if (containsIgnoreCase(t.getArtist(), query))
            results.push_back(t);
    }
    return results;
}

std::vector<Song> Playlist::searchByGenre(const std::string& query) const {
    std::vector<Song> results;
    for (const auto& t : tracks) {
        if (containsIgnoreCase(t.getGenre(), query))
            results.push_back(t);
    }
    return results;
}

// ══════════════════════════════════════════════════════════════════════
// Sorting
// ══════════════════════════════════════════════════════════════════════

void Playlist::sortByTitle() {
    std::sort(tracks.begin(), tracks.end(),
              [](const Song& a, const Song& b) {
                  return a.getTitle() < b.getTitle();
              });
}

void Playlist::sortByArtist() {
    std::sort(tracks.begin(), tracks.end(),
              [](const Song& a, const Song& b) {
                  return a.getArtist() < b.getArtist();
              });
}

void Playlist::sortByDuration() {
    std::sort(tracks.begin(), tracks.end(),
              [](const Song& a, const Song& b) {
                  return a.getDurationSeconds() < b.getDurationSeconds();
              });
}

// ══════════════════════════════════════════════════════════════════════
// Display
// ══════════════════════════════════════════════════════════════════════

void Playlist::displayHeader() const {
    std::cout << "  " << std::left
              << std::setw(4)  << "#"
              << std::setw(5)  << "ID"
              << std::setw(30) << "Title"
              << std::setw(22) << "Artist"
              << std::setw(14) << "Genre"
              << std::right << std::setw(6) << "Time"
              << "\n  " << std::string(81, '-') << std::endl;
}

void Playlist::display() const {
    if (tracks.empty()) {
        std::cout << "\n  [!] Playlist \"" << name << "\" is empty.\n";
        return;
    }
    std::cout << "\n  ========== Playlist: " << name
              << " (" << tracks.size() << " tracks, "
              << getFormattedTotalDuration() << ") ==========\n";
    displayHeader();
    int idx = 1;
    for (const auto& t : tracks) {
        std::cout << "  " << std::left << std::setw(4) << idx;
        t.display();
        ++idx;
    }
    std::cout << "  " << std::string(81, '=') << "\n";
}

const std::vector<Song>& Playlist::getTracks() const {
    return tracks;
}

const Song& Playlist::getTrackAt(int index) const {
    if (index < 0 || index >= static_cast<int>(tracks.size())) {
        throw std::out_of_range("Track index out of range.");
    }
    return tracks[index];
}

// ══════════════════════════════════════════════════════════════════════
// Rebuild (for loading from file)
// ══════════════════════════════════════════════════════════════════════

void Playlist::rebuildFromTracks(const std::string& playlistName,
                                  const std::vector<Song>& loadedTracks) {
    name = playlistName;
    tracks.clear();
    trackIds.clear();
    for (const auto& song : loadedTracks) {
        addSong(song);   // Re-uses addSong to rebuild the ID set
    }
}

// ══════════════════════════════════════════════════════════════════════
// Internal helpers
// ══════════════════════════════════════════════════════════════════════

bool Playlist::containsIgnoreCase(const std::string& haystack,
                                   const std::string& needle) {
    if (needle.empty()) return true;
    std::string lh = haystack, ln = needle;
    std::transform(lh.begin(), lh.end(), lh.begin(), ::tolower);
    std::transform(ln.begin(), ln.end(), ln.begin(), ::tolower);
    return lh.find(ln) != std::string::npos;
}
