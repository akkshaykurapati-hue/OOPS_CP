#ifndef APPLICATION_H
#define APPLICATION_H

#include "MusicLibrary.h"
#include "PlaylistManager.h"
#include "MusicPlayer.h"
#include "PlaylistStorage.h"

/**
 * @class Application
 * @brief Main application controller — coordinates all components.
 *
 * OOP Concepts:
 *   - Composition: Application HAS-A MusicLibrary, PlaylistManager,
 *                  MusicPlayer, and PlaylistStorage.
 *   - Encapsulation: All components are private; interaction is through menus.
 *   - Abstraction: The run() method hides the entire application loop.
 *   - Separation of concerns: Business logic is in domain classes;
 *                              Application handles only menu I/O.
 */
class Application {
private:
    MusicLibrary library;
    PlaylistManager playlistManager;
    MusicPlayer player;
    PlaylistStorage storage;
    bool running;
    int selectedPlaylistIndex;   // -1 = no playlist selected

public:
    Application();

    /// Main entry point — runs the application loop
    void run();

private:
    // ── Menu display ──────────────────────────────────────────────────

    void showBanner() const;
    void showMainMenu() const;
    void showLibraryMenu() const;
    void showPlaylistMenu() const;
    void showPlayerMenu() const;
    void showSearchMenu() const;

    // ── Menu handlers ─────────────────────────────────────────────────

    void handleMainMenu();
    void handleLibraryMenu();
    void handlePlaylistMenu();
    void handlePlayerMenu();
    void handleSearchMenu();

    // ── Library operations ────────────────────────────────────────────

    void addSongToLibrary();
    void removeSongFromLibrary();
    void displayLibrary();
    void searchLibrary();

    // ── Playlist operations ───────────────────────────────────────────

    void createPlaylist();
    void selectPlaylist();
    void addSongToPlaylist();
    void removeSongFromPlaylist();
    void displayPlaylistContents();
    void shufflePlaylist();
    void sortPlaylist();
    void renamePlaylist();
    void deletePlaylist();
    void savePlaylist();
    void loadPlaylist();

    // ── Player operations ─────────────────────────────────────────────

    void loadPlaylistIntoPlayer();

    // ── Search operations ─────────────────────────────────────────────

    void searchInPlaylist();

    // ── Input helpers ─────────────────────────────────────────────────

    /// Read an integer from stdin; returns -1 on invalid input
    int readInt(const std::string& prompt) const;

    /// Read a non-empty string from stdin
    std::string readString(const std::string& prompt) const;

    /// Wait for user to press Enter
    void pressEnterToContinue() const;

    /// Clear the console (best-effort)
    void clearScreen() const;

    /// Display a separator line
    void separator() const;

    /// Display search results
    void displaySearchResults(const std::vector<Song>& results) const;
};

#endif // APPLICATION_H
