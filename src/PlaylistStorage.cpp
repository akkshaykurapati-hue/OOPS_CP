#include "PlaylistStorage.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <stdexcept>

// ══════════════════════════════════════════════════════════════════════
// Constructor
// ══════════════════════════════════════════════════════════════════════

PlaylistStorage::PlaylistStorage() {}

// ══════════════════════════════════════════════════════════════════════
// Save
// ══════════════════════════════════════════════════════════════════════

bool PlaylistStorage::savePlaylist(const Playlist& playlist,
                                    const std::string& filepath) const {
    try {
        std::ofstream file(filepath);
        if (!file.is_open()) {
            std::cerr << "  [ERROR] Could not open file for writing: "
                      << filepath << "\n";
            return false;
        }

        // Line 1: playlist name
        file << "PLAYLIST:" << playlist.getName() << "\n";

        // Subsequent lines: song data in CSV format
        // Format: id,title,artist,genre,durationSeconds
        for (const auto& song : playlist.getTracks()) {
            file << song.getId() << ","
                 << song.getTitle() << ","
                 << song.getArtist() << ","
                 << song.getGenre() << ","
                 << song.getDurationSeconds() << "\n";
        }

        file.close();
        if (file.fail()) {
            std::cerr << "  [ERROR] Write failed for: " << filepath << "\n";
            return false;
        }
        return true;

    } catch (const std::exception& e) {
        std::cerr << "  [ERROR] Exception during save: " << e.what() << "\n";
        return false;
    }
}

// ══════════════════════════════════════════════════════════════════════
// Load
// ══════════════════════════════════════════════════════════════════════

bool PlaylistStorage::loadPlaylist(const std::string& filepath,
                                    Playlist& outPlaylist) const {
    try {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            std::cerr << "  [ERROR] Could not open file: " << filepath << "\n";
            return false;
        }

        std::string line;
        std::string playlistName = "Loaded Playlist";
        std::vector<Song> loadedTracks;
        int lineNum = 0;
        int skippedRecords = 0;

        while (std::getline(file, line)) {
            ++lineNum;
            line = trim(line);
            if (line.empty()) continue;

            // First non-empty line should be the playlist header
            if (lineNum == 1 && line.substr(0, 9) == "PLAYLIST:") {
                playlistName = line.substr(9);
                continue;
            }

            // Parse song data
            Song song;
            if (parseSongLine(line, song)) {
                loadedTracks.push_back(song);
            } else {
                std::cerr << "  [WARNING] Skipped malformed record at line "
                          << lineNum << ": " << line << "\n";
                ++skippedRecords;
            }
        }

        file.close();

        // Rebuild the playlist with loaded data
        // This also rebuilds the unordered_set for duplicate prevention
        outPlaylist.rebuildFromTracks(playlistName, loadedTracks);

        if (skippedRecords > 0) {
            std::cout << "  [!] " << skippedRecords
                      << " malformed record(s) were skipped.\n";
        }

        return true;

    } catch (const std::exception& e) {
        std::cerr << "  [ERROR] Exception during load: " << e.what() << "\n";
        return false;
    }
}

// ══════════════════════════════════════════════════════════════════════
// Static helpers
// ══════════════════════════════════════════════════════════════════════

std::string PlaylistStorage::getDefaultDirectory() {
    return "data/";
}

std::string PlaylistStorage::generateFilename(const std::string& playlistName) {
    std::string filename = playlistName;
    // Replace spaces and special characters with underscores
    for (char& c : filename) {
        if (!std::isalnum(static_cast<unsigned char>(c)) && c != '_' && c != '-') {
            c = '_';
        }
    }
    // Convert to lowercase
    std::transform(filename.begin(), filename.end(), filename.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return filename + ".csv";
}

// ══════════════════════════════════════════════════════════════════════
// Private helpers
// ══════════════════════════════════════════════════════════════════════

bool PlaylistStorage::parseSongLine(const std::string& line, Song& outSong) const {
    /*
     * Expected format: id,title,artist,genre,durationSeconds
     *
     * We use a simple comma-split approach. This works well for our
     * data because song titles, artists, and genres do not contain
     * commas in our sample data. A production system would need a
     * more robust CSV parser (e.g., handling quoted fields).
     */
    std::istringstream iss(line);
    std::string idStr, title, artist, genre, durStr;

    if (!std::getline(iss, idStr,   ',')) return false;
    if (!std::getline(iss, title,   ',')) return false;
    if (!std::getline(iss, artist,  ',')) return false;
    if (!std::getline(iss, genre,   ',')) return false;
    if (!std::getline(iss, durStr,  ',') && !std::getline(iss, durStr)) return false;

    // Trim whitespace
    idStr  = trim(idStr);
    title  = trim(title);
    artist = trim(artist);
    genre  = trim(genre);
    durStr = trim(durStr);

    try {
        int id       = std::stoi(idStr);
        int duration = std::stoi(durStr);

        // Use Song's built-in validation
        Song::validate(id, title, artist, genre, duration);
        outSong = Song(id, title, artist, genre, duration);
        return true;

    } catch (const std::exception&) {
        return false;   // Invalid numeric fields or validation failure
    }
}

std::string PlaylistStorage::trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = str.find_last_not_of(" \t\r\n");
    return str.substr(start, end - start + 1);
}
