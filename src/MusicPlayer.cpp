#include "MusicPlayer.h"
#include <iostream>
#include <iomanip>

// ══════════════════════════════════════════════════════════════════════
// Constructor
// ══════════════════════════════════════════════════════════════════════

MusicPlayer::MusicPlayer()
    : currentPlaylist(nullptr), currentTrackIndex(-1), state(State::STOPPED)
{
}

// ══════════════════════════════════════════════════════════════════════
// Playlist binding
// ══════════════════════════════════════════════════════════════════════

void MusicPlayer::loadPlaylist(const Playlist* playlist) {
    currentPlaylist = playlist;
    currentTrackIndex = -1;
    state = State::STOPPED;
}

bool MusicPlayer::hasPlaylist() const {
    return currentPlaylist != nullptr && !currentPlaylist->isEmpty();
}

// ══════════════════════════════════════════════════════════════════════
// Playable interface overrides — POLYMORPHISM
// ══════════════════════════════════════════════════════════════════════

void MusicPlayer::play() {
    if (!hasPlaylist()) {
        std::cout << "  [!] No playlist loaded or playlist is empty.\n";
        return;
    }
    currentTrackIndex = 0;
    state = State::PLAYING;
    std::cout << "\n  >> Now playing: "
              << currentPlaylist->getTrackAt(currentTrackIndex).getTitle()
              << " - "
              << currentPlaylist->getTrackAt(currentTrackIndex).getArtist()
              << " ["
              << currentPlaylist->getTrackAt(currentTrackIndex).getFormattedDuration()
              << "]  (SIMULATED)\n";
}

void MusicPlayer::pause() {
    if (state != State::PLAYING) {
        std::cout << "  [!] Nothing is currently playing.\n";
        return;
    }
    state = State::PAUSED;
    std::cout << "  || Paused: "
              << currentPlaylist->getTrackAt(currentTrackIndex).getTitle()
              << "\n";
}

void MusicPlayer::resume() {
    if (state != State::PAUSED) {
        std::cout << "  [!] Playback is not paused.\n";
        return;
    }
    state = State::PLAYING;
    std::cout << "  >> Resumed: "
              << currentPlaylist->getTrackAt(currentTrackIndex).getTitle()
              << " - "
              << currentPlaylist->getTrackAt(currentTrackIndex).getArtist()
              << "  (SIMULATED)\n";
}

void MusicPlayer::next() {
    if (!hasPlaylist()) {
        std::cout << "  [!] No playlist loaded or playlist is empty.\n";
        return;
    }
    if (currentTrackIndex < 0) {
        // Player hasn't started yet — start from the beginning
        play();
        return;
    }
    if (currentTrackIndex >= currentPlaylist->getTrackCount() - 1) {
        std::cout << "  [!] Already at the last track in the playlist.\n";
        return;
    }
    ++currentTrackIndex;
    state = State::PLAYING;
    std::cout << "\n  >> Next track: "
              << currentPlaylist->getTrackAt(currentTrackIndex).getTitle()
              << " - "
              << currentPlaylist->getTrackAt(currentTrackIndex).getArtist()
              << " ["
              << currentPlaylist->getTrackAt(currentTrackIndex).getFormattedDuration()
              << "]  (SIMULATED)\n";
}

void MusicPlayer::previous() {
    if (!hasPlaylist()) {
        std::cout << "  [!] No playlist loaded or playlist is empty.\n";
        return;
    }
    if (currentTrackIndex <= 0) {
        std::cout << "  [!] Already at the first track in the playlist.\n";
        return;
    }
    --currentTrackIndex;
    state = State::PLAYING;
    std::cout << "\n  << Previous track: "
              << currentPlaylist->getTrackAt(currentTrackIndex).getTitle()
              << " - "
              << currentPlaylist->getTrackAt(currentTrackIndex).getArtist()
              << " ["
              << currentPlaylist->getTrackAt(currentTrackIndex).getFormattedDuration()
              << "]  (SIMULATED)\n";
}

void MusicPlayer::showStatus() const {
    std::cout << "\n  +--- Player Status ----------------------------+\n";
    std::cout << "  |  State: " << getStateString() << "\n";

    if (currentPlaylist != nullptr) {
        std::cout << "  |  Playlist: " << currentPlaylist->getName()
                  << " (" << currentPlaylist->getTrackCount() << " tracks)\n";
    } else {
        std::cout << "  |  Playlist: (none loaded)\n";
    }

    if (currentTrackIndex >= 0 && currentPlaylist != nullptr
        && currentTrackIndex < currentPlaylist->getTrackCount()) {
        const Song& s = currentPlaylist->getTrackAt(currentTrackIndex);
        std::cout << "  |  Track " << (currentTrackIndex + 1)
                  << "/" << currentPlaylist->getTrackCount() << ": "
                  << s.getTitle() << " - " << s.getArtist()
                  << " [" << s.getFormattedDuration() << "]\n";
    } else {
        std::cout << "  |  Track: (none)\n";
    }
    std::cout << "  |  NOTE: Playback is simulated (no audio output)\n";
    std::cout << "  +----------------------------------------------+\n";
}

// ══════════════════════════════════════════════════════════════════════
// State queries
// ══════════════════════════════════════════════════════════════════════

MusicPlayer::State MusicPlayer::getState() const { return state; }

std::string MusicPlayer::getStateString() const {
    switch (state) {
        case State::STOPPED: return "STOPPED";
        case State::PLAYING: return "PLAYING";
        case State::PAUSED:  return "PAUSED";
        default:             return "UNKNOWN";
    }
}

int MusicPlayer::getCurrentTrackIndex() const { return currentTrackIndex; }

const Song* MusicPlayer::getCurrentSong() const {
    if (currentPlaylist == nullptr || currentTrackIndex < 0
        || currentTrackIndex >= currentPlaylist->getTrackCount()) {
        return nullptr;
    }
    return &currentPlaylist->getTrackAt(currentTrackIndex);
}
