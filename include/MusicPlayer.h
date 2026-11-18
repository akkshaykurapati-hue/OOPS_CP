#ifndef MUSIC_PLAYER_H
#define MUSIC_PLAYER_H

#include "Playable.h"
#include "Playlist.h"
#include <string>

/**
 * @class MusicPlayer
 * @brief Simulates audio playback for a given playlist.
 *
 * OOP Concepts:
 *   - Inheritance: MusicPlayer IS-A Playable (derives from abstract class).
 *   - Polymorphism: Overrides all pure virtual methods from Playable.
 *   - Association: MusicPlayer references a Playlist via pointer (does NOT own it).
 *   - Encapsulation: Playback state is private.
 *
 * IMPORTANT: This is a *simulated* player. It does NOT play actual audio files.
 * It tracks which song is "playing", maintains play/pause state, and supports
 * next/previous navigation.
 */
class MusicPlayer : public Playable {
public:
    /// Playback state enumeration
    enum class State {
        STOPPED,
        PLAYING,
        PAUSED
    };

private:
    const Playlist* currentPlaylist;   // Non-owning pointer to the active playlist
    int currentTrackIndex;             // 0-based index into the playlist's tracks
    State state;                       // Current playback state

public:
    // ── Constructors ──────────────────────────────────────────────────

    MusicPlayer();

    // ── Playlist binding ──────────────────────────────────────────────

    /// Load a playlist into the player; resets playback state
    void loadPlaylist(const Playlist* playlist);

    /// Check if a playlist is loaded
    bool hasPlaylist() const;

    // ── Playable interface overrides (polymorphism) ───────────────────

    void play() override;
    void pause() override;
    void resume() override;
    void next() override;
    void previous() override;
    void showStatus() const override;

    // ── State queries ─────────────────────────────────────────────────

    State getState() const;
    std::string getStateString() const;
    int getCurrentTrackIndex() const;

    /// Get the currently active song; returns nullptr if none
    const Song* getCurrentSong() const;
};

#endif // MUSIC_PLAYER_H
