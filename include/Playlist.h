#ifndef PLAYLIST_H
#define PLAYLIST_H

#include "Song.h"
#include <vector>
#include <unordered_set>
#include <string>
#include <random>
#include <algorithm>
#include <functional>

/**
 * @class Playlist
 * @brief Represents an ordered collection of songs with duplicate prevention.
 *
 * This class is the core data structure mandated by the official project
 * requirements. It uses:
 *   - std::vector<Song>        : ordered storage (ArrayList equivalent)
 *   - std::unordered_set<int>  : O(1) average duplicate ID detection (HashSet equivalent)
 *
 * OOP Concepts:
 *   - Encapsulation: Both containers are private; access is through public methods.
 *   - Abstraction: addSong() hides the dual-structure insert logic.
 *   - Composition: A Playlist HAS-A vector of Songs and a set of IDs.
 *   - Const correctness: Read-only methods are const-qualified.
 */
class Playlist {
private:
    std::string name;                   // Playlist display name
    std::vector<Song> tracks;           // Ordered track list
    std::unordered_set<int> trackIds;   // Set of IDs for duplicate prevention

public:
    // ── Constructors ──────────────────────────────────────────────────

    Playlist();
    explicit Playlist(const std::string& name);

    // ── Getters ───────────────────────────────────────────────────────

    std::string getName() const;
    int getTrackCount() const;
    bool isEmpty() const;

    // ── Setters ───────────────────────────────────────────────────────

    void setName(const std::string& newName);

    // ── Core operations (official requirements) ───────────────────────

    /// Add a song; returns false if duplicate ID detected
    bool addSong(const Song& song);

    /// Update a song in place if it exists in this playlist
    bool updateSong(const Song& song);

    /// Remove a song by ID; returns false if not found
    bool removeSong(int id);

    /// Check if track ID exists in this playlist
    bool containsSong(int id) const;

    /// Calculate total playlist duration in seconds
    int getTotalDuration() const;

    /// Get formatted total duration as "HH:MM:SS"
    std::string getFormattedTotalDuration() const;

    // ── Shuffling (official requirement) ──────────────────────────────

    /// Shuffle tracks using std::shuffle and std::mt19937
    void shuffle();

    /// Shuffle with a specific seed (for reproducible tests)
    void shuffle(unsigned int seed);

    // ── Search ────────────────────────────────────────────────────────

    const Song* findById(int id) const;
    std::vector<Song> searchByTitle(const std::string& query) const;
    std::vector<Song> searchByArtist(const std::string& query) const;
    std::vector<Song> searchByGenre(const std::string& query) const;

    // ── Sorting ───────────────────────────────────────────────────────

    void sortByTitle();
    void sortByArtist();
    void sortByDuration();

    // ── Display ───────────────────────────────────────────────────────

    void display() const;
    void displayHeader() const;

    // ── Access ────────────────────────────────────────────────────────

    /// Read-only access to the tracks vector
    const std::vector<Song>& getTracks() const;

    /// Access a track by index (0-based); throws if out of range
    const Song& getTrackAt(int index) const;

    // ── Rebuild (used when loading from file) ─────────────────────────

    /// Clears the playlist and re-populates from a vector of songs
    void rebuildFromTracks(const std::string& playlistName,
                           const std::vector<Song>& loadedTracks);

private:
    /// Case-insensitive substring match
    static bool containsIgnoreCase(const std::string& haystack,
                                   const std::string& needle);
};

#endif // PLAYLIST_H
