#ifndef PLAYLIST_STORAGE_H
#define PLAYLIST_STORAGE_H

#include "Playlist.h"
#include "MusicLibrary.h"
#include <string>

/**
 * @class PlaylistStorage
 * @brief Handles file persistence for playlists — save and load operations.
 *
 * OOP Concepts:
 *   - Separation of concerns: File I/O is isolated from Playlist logic.
 *   - Encapsulation: File format details are hidden from other classes.
 *   - Exception handling: File errors are caught and reported gracefully.
 *
 * File Format (CSV):
 *   Line 1: PLAYLIST:<playlist_name>
 *   Lines 2+: <id>,<title>,<artist>,<genre>,<durationSeconds>
 *
 * Design Decision:
 *   Saving a playlist writes the track data directly. The master library
 *   is NOT modified by save/load operations. When loading, songs are
 *   reconstructed from the file, and the playlist's internal
 *   unordered_set is rebuilt via rebuildFromTracks().
 */
class PlaylistStorage {
public:
    PlaylistStorage();

    /// Save a playlist to a file
    /// @param playlist The playlist to save
    /// @param filepath Path to the output file
    /// @return true on success, false on failure
    bool savePlaylist(const Playlist& playlist,
                      const std::string& filepath) const;

    /// Load a playlist from a file
    /// @param filepath Path to the input file
    /// @param outPlaylist The playlist object to populate
    /// @return true on success, false on failure
    bool loadPlaylist(const std::string& filepath,
                      Playlist& outPlaylist) const;

    /// Get the default save directory
    static std::string getDefaultDirectory();

    /// Generate a filename from a playlist name
    static std::string generateFilename(const std::string& playlistName);

private:
    /// Parse a single CSV line into a Song object
    /// @return true if parsing succeeded, false if the record is malformed
    bool parseSongLine(const std::string& line, Song& outSong) const;

    /// Trim leading and trailing whitespace
    static std::string trim(const std::string& str);
};

#endif // PLAYLIST_STORAGE_H
