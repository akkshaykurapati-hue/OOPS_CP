#ifndef MUSIC_LIBRARY_H
#define MUSIC_LIBRARY_H

#include "Song.h"
#include <vector>
#include <unordered_set>
#include <string>
#include <algorithm>

/**
 * @class MusicLibrary
 * @brief Maintains the master collection of all songs in the application.
 *
 * OOP Concepts:
 *   - Encapsulation: songs vector and ID set are private.
 *   - Abstraction: public methods hide internal storage details.
 *   - Composition: MusicLibrary HAS-A collection of Song objects.
 *
 * Data Structures:
 *   - std::vector<Song> : ordered storage of all songs.
 *   - std::unordered_set<int> : O(1) average-case duplicate-ID detection.
 */
class MusicLibrary {
private:
    std::vector<Song> songs;           // Ordered master list
    std::unordered_set<int> songIds;   // Fast ID lookup for duplicate prevention

public:
    // ── Constructor ───────────────────────────────────────────────────
    MusicLibrary();

    // ── Core operations ───────────────────────────────────────────────

    /// Add a song; returns false if ID already exists
    bool addSong(const Song& song);

    /// Update an existing song; returns false if not found
    bool updateSong(const Song& song);

    /// Remove a song by ID; returns false if not found
    bool removeSong(int id);

    /// Clear all songs from library
    void clear();

    /// Check if a song ID exists in the library
    bool containsSong(int id) const;

    /// Get total number of songs
    int getSongCount() const;

    // ── Search operations ─────────────────────────────────────────────

    /// Find a song by ID; returns nullptr if not found
    const Song* findById(int id) const;

    /// Find songs whose title contains the query (case-insensitive)
    std::vector<Song> searchByTitle(const std::string& query) const;

    /// Find songs whose artist contains the query (case-insensitive)
    std::vector<Song> searchByArtist(const std::string& query) const;

    /// Find songs whose genre contains the query (case-insensitive)
    std::vector<Song> searchByGenre(const std::string& query) const;

    // ── Display ───────────────────────────────────────────────────────

    /// Display all songs in a formatted table
    void displayAll() const;

    /// Display the column header for the song table
    void displayHeader() const;

    // ── Access ────────────────────────────────────────────────────────

    /// Provide read-only access to the songs vector
    const std::vector<Song>& getAllSongs() const;

    // ── Sample data ───────────────────────────────────────────────────

    /// Load pre-built sample tracks into the library
    void loadSampleData();

private:
    // ── Internal helpers ──────────────────────────────────────────────

    /// Case-insensitive substring match
    static bool containsIgnoreCase(const std::string& haystack,
                                   const std::string& needle);
};

#endif // MUSIC_LIBRARY_H
