#ifndef PLAYLIST_MANAGER_H
#define PLAYLIST_MANAGER_H

#include "Playlist.h"
#include <vector>
#include <string>

/**
 * @class PlaylistManager
 * @brief Manages multiple Playlist objects — create, select, rename, delete.
 *
 * OOP Concepts:
 *   - Composition: PlaylistManager HAS-A vector of Playlist objects.
 *   - Encapsulation: The playlists vector is private.
 *   - Abstraction: Simple public interface hides container operations.
 *
 * Data Structure Choice:
 *   std::vector<Playlist> is used because:
 *   1. Playlists are accessed by index during menu selection.
 *   2. The expected number of playlists is small (dozens, not millions).
 *   3. Vector provides cache-friendly sequential access.
 */
class PlaylistManager {
private:
    std::vector<Playlist> playlists;

public:
    PlaylistManager();

    // ── Playlist management ───────────────────────────────────────────

    /// Create a new playlist; returns false if name already exists
    bool createPlaylist(const std::string& name);

    /// Delete a playlist by index (0-based); returns false if invalid
    bool deletePlaylist(int index);

    /// Rename a playlist by index; returns false if invalid or name taken
    bool renamePlaylist(int index, const std::string& newName);

    /// Get a mutable reference to a playlist by index
    Playlist& getPlaylist(int index);

    /// Get a const reference to a playlist by index
    const Playlist& getPlaylistConst(int index) const;

    /// Get total number of playlists
    int getPlaylistCount() const;

    /// Check if a playlist name already exists
    bool nameExists(const std::string& name) const;

    /// Find a playlist index by name; returns -1 if not found
    int findByName(const std::string& name) const;

    // ── Display ───────────────────────────────────────────────────────

    /// List all playlists with index, name, and track count
    void displayAll() const;

    // ── Direct access ─────────────────────────────────────────────────

    /// Add a pre-built playlist (used when loading from file)
    void addPlaylist(const Playlist& playlist);

    /// Remove a song by ID from all managed playlists
    void removeSongFromAllPlaylists(int songId);

    /// Update song metadata in all managed playlists
    void updateSongInAllPlaylists(const Song& updatedSong);

    /// Clear all playlists
    void clear();
};

#endif // PLAYLIST_MANAGER_H
