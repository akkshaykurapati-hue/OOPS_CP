#include "PlaylistManager.h"
#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <algorithm>

// ══════════════════════════════════════════════════════════════════════
// Constructor
// ══════════════════════════════════════════════════════════════════════

PlaylistManager::PlaylistManager() {}

// ══════════════════════════════════════════════════════════════════════
// Playlist management
// ══════════════════════════════════════════════════════════════════════

bool PlaylistManager::createPlaylist(const std::string& name) {
    if (name.empty()) return false;
    if (nameExists(name)) return false;
    playlists.push_back(Playlist(name));
    return true;
}

bool PlaylistManager::deletePlaylist(int index) {
    if (index < 0 || index >= static_cast<int>(playlists.size())) {
        return false;
    }
    playlists.erase(playlists.begin() + index);
    return true;
}

bool PlaylistManager::renamePlaylist(int index, const std::string& newName) {
    if (index < 0 || index >= static_cast<int>(playlists.size())) {
        return false;
    }
    if (newName.empty()) return false;
    if (nameExists(newName)) return false;
    playlists[index].setName(newName);
    return true;
}

Playlist& PlaylistManager::getPlaylist(int index) {
    if (index < 0 || index >= static_cast<int>(playlists.size())) {
        throw std::out_of_range("Playlist index out of range.");
    }
    return playlists[index];
}

const Playlist& PlaylistManager::getPlaylistConst(int index) const {
    if (index < 0 || index >= static_cast<int>(playlists.size())) {
        throw std::out_of_range("Playlist index out of range.");
    }
    return playlists[index];
}

int PlaylistManager::getPlaylistCount() const {
    return static_cast<int>(playlists.size());
}

bool PlaylistManager::nameExists(const std::string& name) const {
    for (const auto& pl : playlists) {
        // Case-sensitive name comparison
        if (pl.getName() == name) return true;
    }
    return false;
}

int PlaylistManager::findByName(const std::string& name) const {
    for (int i = 0; i < static_cast<int>(playlists.size()); ++i) {
        if (playlists[i].getName() == name) return i;
    }
    return -1;
}

// ══════════════════════════════════════════════════════════════════════
// Display
// ══════════════════════════════════════════════════════════════════════

void PlaylistManager::displayAll() const {
    if (playlists.empty()) {
        std::cout << "\n  [!] No playlists created yet.\n";
        return;
    }
    std::cout << "\n  ============= YOUR PLAYLISTS =============\n";
    std::cout << "  " << std::left
              << std::setw(5)  << "#"
              << std::setw(30) << "Playlist Name"
              << std::setw(10) << "Tracks"
              << "Duration\n";
    std::cout << "  " << std::string(55, '-') << "\n";
    for (int i = 0; i < static_cast<int>(playlists.size()); ++i) {
        std::cout << "  " << std::left
                  << std::setw(5)  << (i + 1)
                  << std::setw(30) << playlists[i].getName()
                  << std::setw(10) << playlists[i].getTrackCount()
                  << playlists[i].getFormattedTotalDuration() << "\n";
    }
    std::cout << "  " << std::string(55, '=') << "\n";
}

// ══════════════════════════════════════════════════════════════════════
// Direct access
// ══════════════════════════════════════════════════════════════════════

void PlaylistManager::addPlaylist(const Playlist& playlist) {
    playlists.push_back(playlist);
}

void PlaylistManager::removeSongFromAllPlaylists(int songId) {
    for (auto& pl : playlists) {
        pl.removeSong(songId);
    }
}

void PlaylistManager::updateSongInAllPlaylists(const Song& updatedSong) {
    for (auto& pl : playlists) {
        pl.updateSong(updatedSong);
    }
}

void PlaylistManager::clear() {
    playlists.clear();
}
