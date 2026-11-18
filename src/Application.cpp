#include "Application.h"
#include <iostream>
#include <limits>
#include <sstream>
#include <cstdlib>

// ══════════════════════════════════════════════════════════════════════
// Constructor
// ══════════════════════════════════════════════════════════════════════

Application::Application()
    : running(true), selectedPlaylistIndex(-1)
{
    // Load sample data into the music library
    library.loadSampleData();
}

// ══════════════════════════════════════════════════════════════════════
// Main application loop
// ══════════════════════════════════════════════════════════════════════

void Application::run() {
    showBanner();
    std::cout << "  Sample music library loaded with "
              << library.getSongCount() << " tracks.\n\n";
    pressEnterToContinue();

    while (running) {
        clearScreen();
        showMainMenu();
        handleMainMenu();
    }

    std::cout << "\n  Thank you for using Smart Playlist Synthesizer!\n"
              << "  Goodbye.\n\n";
}

// ══════════════════════════════════════════════════════════════════════
// Banner and Menus
// ══════════════════════════════════════════════════════════════════════

void Application::showBanner() const {
    std::cout << "\n";
    std::cout << "  ╔══════════════════════════════════════════════════════╗\n";
    std::cout << "  ║       SMART PLAYLIST SYNTHESIZER                    ║\n";
    std::cout << "  ║       Music Library & Playlist Manager              ║\n";
    std::cout << "  ║                                                      ║\n";
    std::cout << "  ║       OOP Course Project — SY Engineering           ║\n";
    std::cout << "  ╚══════════════════════════════════════════════════════╝\n";
    std::cout << "\n";
}

void Application::showMainMenu() const {
    showBanner();

    // Show selected playlist indicator
    if (selectedPlaylistIndex >= 0 &&
        selectedPlaylistIndex < playlistManager.getPlaylistCount()) {
        std::cout << "  [Active Playlist: "
                  << playlistManager.getPlaylistConst(selectedPlaylistIndex).getName()
                  << "]\n";
    }

    std::cout << "\n  ┌─────────── MAIN MENU ───────────┐\n";
    std::cout << "  │  1.  Music Library               │\n";
    std::cout << "  │  2.  Playlist Management          │\n";
    std::cout << "  │  3.  Music Player                 │\n";
    std::cout << "  │  4.  Search & Sort                │\n";
    std::cout << "  │  5.  Save Playlist to File        │\n";
    std::cout << "  │  6.  Load Playlist from File      │\n";
    std::cout << "  │  0.  Exit                         │\n";
    std::cout << "  └───────────────────────────────────┘\n";
}

void Application::showLibraryMenu() const {
    std::cout << "\n  ┌─────── MUSIC LIBRARY ────────┐\n";
    std::cout << "  │  1.  View All Songs           │\n";
    std::cout << "  │  2.  Add a New Song           │\n";
    std::cout << "  │  3.  Remove a Song            │\n";
    std::cout << "  │  4.  Search Library           │\n";
    std::cout << "  │  0.  Back to Main Menu        │\n";
    std::cout << "  └───────────────────────────────┘\n";
}

void Application::showPlaylistMenu() const {
    std::cout << "\n";
    if (selectedPlaylistIndex >= 0 &&
        selectedPlaylistIndex < playlistManager.getPlaylistCount()) {
        std::cout << "  [Active: "
                  << playlistManager.getPlaylistConst(selectedPlaylistIndex).getName()
                  << "]\n";
    }
    std::cout << "  ┌────── PLAYLIST MANAGEMENT ──────┐\n";
    std::cout << "  │  1.  Create New Playlist         │\n";
    std::cout << "  │  2.  View All Playlists          │\n";
    std::cout << "  │  3.  Select a Playlist           │\n";
    std::cout << "  │  4.  Add Song to Playlist        │\n";
    std::cout << "  │  5.  Remove Song from Playlist   │\n";
    std::cout << "  │  6.  Display Playlist Contents   │\n";
    std::cout << "  │  7.  Shuffle Playlist             │\n";
    std::cout << "  │  8.  Sort Playlist                │\n";
    std::cout << "  │  9.  Rename Playlist              │\n";
    std::cout << "  │ 10.  Delete Playlist              │\n";
    std::cout << "  │  0.  Back to Main Menu            │\n";
    std::cout << "  └───────────────────────────────────┘\n";
}

void Application::showPlayerMenu() const {
    std::cout << "\n  ┌──────── MUSIC PLAYER ─────────┐\n";
    std::cout << "  │  1.  Load Playlist into Player │\n";
    std::cout << "  │  2.  Play                      │\n";
    std::cout << "  │  3.  Pause                     │\n";
    std::cout << "  │  4.  Resume                    │\n";
    std::cout << "  │  5.  Next Track                │\n";
    std::cout << "  │  6.  Previous Track            │\n";
    std::cout << "  │  7.  Show Player Status        │\n";
    std::cout << "  │  0.  Back to Main Menu         │\n";
    std::cout << "  └────────────────────────────────┘\n";
    std::cout << "  (Note: Playback is simulated)\n";
}

void Application::showSearchMenu() const {
    std::cout << "\n";
    if (selectedPlaylistIndex >= 0 &&
        selectedPlaylistIndex < playlistManager.getPlaylistCount()) {
        std::cout << "  [Searching in: "
                  << playlistManager.getPlaylistConst(selectedPlaylistIndex).getName()
                  << "]\n";
    }
    std::cout << "  ┌──────── SEARCH & SORT ────────┐\n";
    std::cout << "  │  1.  Search Library by Title   │\n";
    std::cout << "  │  2.  Search Library by Artist  │\n";
    std::cout << "  │  3.  Search Library by Genre   │\n";
    std::cout << "  │  4.  Search Playlist by Title  │\n";
    std::cout << "  │  5.  Search Playlist by Artist │\n";
    std::cout << "  │  6.  Search Playlist by Genre  │\n";
    std::cout << "  │  7.  Sort Playlist by Title    │\n";
    std::cout << "  │  8.  Sort Playlist by Artist   │\n";
    std::cout << "  │  9.  Sort Playlist by Duration │\n";
    std::cout << "  │  0.  Back to Main Menu         │\n";
    std::cout << "  └────────────────────────────────┘\n";
}

// ══════════════════════════════════════════════════════════════════════
// Menu handlers
// ══════════════════════════════════════════════════════════════════════

void Application::handleMainMenu() {
    int choice = readInt("  Enter your choice: ");
    switch (choice) {
        case 1: handleLibraryMenu();   break;
        case 2: handlePlaylistMenu();  break;
        case 3: handlePlayerMenu();    break;
        case 4: handleSearchMenu();    break;
        case 5: savePlaylist();        break;
        case 6: loadPlaylist();        break;
        case 0:
            running = false;
            break;
        default:
            std::cout << "  [!] Invalid choice. Please enter 0-6.\n";
            pressEnterToContinue();
    }
}

void Application::handleLibraryMenu() {
    bool inLibrary = true;
    while (inLibrary) {
        showLibraryMenu();
        int choice = readInt("  Enter your choice: ");
        switch (choice) {
            case 1: displayLibrary();        break;
            case 2: addSongToLibrary();      break;
            case 3: removeSongFromLibrary(); break;
            case 4: searchLibrary();         break;
            case 0: inLibrary = false;       break;
            default:
                std::cout << "  [!] Invalid choice.\n";
        }
        if (inLibrary) pressEnterToContinue();
    }
}

void Application::handlePlaylistMenu() {
    bool inPlaylist = true;
    while (inPlaylist) {
        showPlaylistMenu();
        int choice = readInt("  Enter your choice: ");
        switch (choice) {
            case 1:  createPlaylist();          break;
            case 2:  playlistManager.displayAll(); break;
            case 3:  selectPlaylist();          break;
            case 4:  addSongToPlaylist();       break;
            case 5:  removeSongFromPlaylist();  break;
            case 6:  displayPlaylistContents(); break;
            case 7:  shufflePlaylist();         break;
            case 8:  sortPlaylist();            break;
            case 9:  renamePlaylist();          break;
            case 10: deletePlaylist();          break;
            case 0:  inPlaylist = false;        break;
            default:
                std::cout << "  [!] Invalid choice.\n";
        }
        if (inPlaylist) pressEnterToContinue();
    }
}

void Application::handlePlayerMenu() {
    bool inPlayer = true;
    while (inPlayer) {
        showPlayerMenu();
        int choice = readInt("  Enter your choice: ");

        // Use base-class pointer to demonstrate polymorphism
        Playable* playable = &player;

        switch (choice) {
            case 1: loadPlaylistIntoPlayer(); break;
            case 2: playable->play();         break;
            case 3: playable->pause();        break;
            case 4: playable->resume();       break;
            case 5: playable->next();         break;
            case 6: playable->previous();     break;
            case 7: playable->showStatus();   break;
            case 0: inPlayer = false;         break;
            default:
                std::cout << "  [!] Invalid choice.\n";
        }
        if (inPlayer) pressEnterToContinue();
    }
}

void Application::handleSearchMenu() {
    bool inSearch = true;
    while (inSearch) {
        showSearchMenu();
        int choice = readInt("  Enter your choice: ");
        switch (choice) {
            case 1: {
                std::string q = readString("  Enter title to search: ");
                displaySearchResults(library.searchByTitle(q));
                break;
            }
            case 2: {
                std::string q = readString("  Enter artist to search: ");
                displaySearchResults(library.searchByArtist(q));
                break;
            }
            case 3: {
                std::string q = readString("  Enter genre to search: ");
                displaySearchResults(library.searchByGenre(q));
                break;
            }
            case 4: case 5: case 6: {
                if (selectedPlaylistIndex < 0) {
                    std::cout << "  [!] No playlist selected. Select one first.\n";
                    break;
                }
                const Playlist& pl = playlistManager.getPlaylistConst(selectedPlaylistIndex);
                std::string q;
                std::vector<Song> results;
                if (choice == 4) {
                    q = readString("  Enter title to search: ");
                    results = pl.searchByTitle(q);
                } else if (choice == 5) {
                    q = readString("  Enter artist to search: ");
                    results = pl.searchByArtist(q);
                } else {
                    q = readString("  Enter genre to search: ");
                    results = pl.searchByGenre(q);
                }
                displaySearchResults(results);
                break;
            }
            case 7: case 8: case 9: {
                if (selectedPlaylistIndex < 0) {
                    std::cout << "  [!] No playlist selected.\n";
                    break;
                }
                Playlist& pl = playlistManager.getPlaylist(selectedPlaylistIndex);
                if (choice == 7) {
                    pl.sortByTitle();
                    std::cout << "  [OK] Playlist sorted by title.\n";
                } else if (choice == 8) {
                    pl.sortByArtist();
                    std::cout << "  [OK] Playlist sorted by artist.\n";
                } else {
                    pl.sortByDuration();
                    std::cout << "  [OK] Playlist sorted by duration.\n";
                }
                pl.display();
                break;
            }
            case 0: inSearch = false; break;
            default:
                std::cout << "  [!] Invalid choice.\n";
        }
        if (inSearch) pressEnterToContinue();
    }
}

// ══════════════════════════════════════════════════════════════════════
// Library operations
// ══════════════════════════════════════════════════════════════════════

void Application::displayLibrary() {
    library.displayAll();
}

void Application::addSongToLibrary() {
    std::cout << "\n  --- Add a New Song to Library ---\n";
    int id = readInt("  Enter song ID (positive integer): ");
    if (id <= 0) {
        std::cout << "  [!] Invalid ID. Must be a positive integer.\n";
        return;
    }
    std::string title  = readString("  Enter song title: ");
    std::string artist = readString("  Enter artist name: ");
    std::string genre  = readString("  Enter genre: ");
    int duration = readInt("  Enter duration in seconds: ");
    if (duration <= 0) {
        std::cout << "  [!] Duration must be positive.\n";
        return;
    }

    try {
        Song newSong(id, title, artist, genre, duration);
        if (library.addSong(newSong)) {
            std::cout << "  [OK] Song added successfully!\n";
            newSong.displayDetailed();
        } else {
            std::cout << "  [!] A song with ID " << id
                      << " already exists in the library.\n";
        }
    } catch (const std::invalid_argument& e) {
        std::cout << "  [ERROR] " << e.what() << "\n";
    }
}

void Application::removeSongFromLibrary() {
    std::cout << "\n  --- Remove a Song from Library ---\n";
    library.displayAll();
    int id = readInt("  Enter the ID of the song to remove: ");
    if (library.removeSong(id)) {
        std::cout << "  [OK] Song with ID " << id << " removed.\n";
    } else {
        std::cout << "  [!] No song found with ID " << id << ".\n";
    }
}

void Application::searchLibrary() {
    std::cout << "\n  --- Search Library ---\n";
    std::cout << "  1. Search by ID\n";
    std::cout << "  2. Search by Title\n";
    std::cout << "  3. Search by Artist\n";
    std::cout << "  4. Search by Genre\n";
    int choice = readInt("  Enter your choice: ");

    switch (choice) {
        case 1: {
            int id = readInt("  Enter song ID: ");
            const Song* s = library.findById(id);
            if (s) {
                s->displayDetailed();
            } else {
                std::cout << "  [!] No song found with ID " << id << ".\n";
            }
            break;
        }
        case 2: {
            std::string q = readString("  Enter title keyword: ");
            displaySearchResults(library.searchByTitle(q));
            break;
        }
        case 3: {
            std::string q = readString("  Enter artist keyword: ");
            displaySearchResults(library.searchByArtist(q));
            break;
        }
        case 4: {
            std::string q = readString("  Enter genre keyword: ");
            displaySearchResults(library.searchByGenre(q));
            break;
        }
        default:
            std::cout << "  [!] Invalid choice.\n";
    }
}

// ══════════════════════════════════════════════════════════════════════
// Playlist operations
// ══════════════════════════════════════════════════════════════════════

void Application::createPlaylist() {
    std::string name = readString("  Enter playlist name: ");
    if (playlistManager.createPlaylist(name)) {
        std::cout << "  [OK] Playlist \"" << name << "\" created.\n";
        // Auto-select the newly created playlist
        selectedPlaylistIndex = playlistManager.getPlaylistCount() - 1;
        std::cout << "  [*] Playlist \"" << name << "\" is now active.\n";
    } else {
        std::cout << "  [!] Could not create playlist. Name may already exist or is empty.\n";
    }
}

void Application::selectPlaylist() {
    if (playlistManager.getPlaylistCount() == 0) {
        std::cout << "  [!] No playlists available. Create one first.\n";
        return;
    }
    playlistManager.displayAll();
    int idx = readInt("  Enter playlist number to select: ");
    idx -= 1;  // Convert to 0-based
    if (idx >= 0 && idx < playlistManager.getPlaylistCount()) {
        selectedPlaylistIndex = idx;
        std::cout << "  [OK] Selected playlist: "
                  << playlistManager.getPlaylistConst(idx).getName() << "\n";
    } else {
        std::cout << "  [!] Invalid playlist number.\n";
    }
}

void Application::addSongToPlaylist() {
    if (selectedPlaylistIndex < 0 ||
        selectedPlaylistIndex >= playlistManager.getPlaylistCount()) {
        std::cout << "  [!] No playlist selected. Please select or create one first.\n";
        return;
    }

    library.displayAll();
    int songId = readInt("  Enter the ID of the song to add: ");

    const Song* song = library.findById(songId);
    if (song == nullptr) {
        std::cout << "  [!] No song found with ID " << songId
                  << " in the library.\n";
        return;
    }

    Playlist& pl = playlistManager.getPlaylist(selectedPlaylistIndex);

    /*
     * DUPLICATE PREVENTION (Official Requirement):
     * pl.addSong() internally checks the unordered_set for the song's ID.
     * If found, it returns false without modifying the vector.
     * If not found, it inserts the ID into the set AND appends to the vector.
     */
    if (pl.addSong(*song)) {
        std::cout << "  [OK] \"" << song->getTitle()
                  << "\" added to playlist \"" << pl.getName() << "\".\n";
    } else {
        std::cout << "  [!] DUPLICATE DETECTED: \"" << song->getTitle()
                  << "\" (ID " << songId
                  << ") is already in playlist \"" << pl.getName() << "\".\n"
                  << "       The song was NOT added again.\n"
                  << "       (Duplicate prevention uses std::unordered_set for O(1) lookup)\n";
    }
}

void Application::removeSongFromPlaylist() {
    if (selectedPlaylistIndex < 0) {
        std::cout << "  [!] No playlist selected.\n";
        return;
    }
    Playlist& pl = playlistManager.getPlaylist(selectedPlaylistIndex);
    if (pl.isEmpty()) {
        std::cout << "  [!] Playlist is empty.\n";
        return;
    }
    pl.display();
    int id = readInt("  Enter the ID of the song to remove: ");
    if (pl.removeSong(id)) {
        std::cout << "  [OK] Song with ID " << id << " removed from playlist.\n";
    } else {
        std::cout << "  [!] No song with ID " << id << " found in this playlist.\n";
    }
}

void Application::displayPlaylistContents() {
    if (selectedPlaylistIndex < 0) {
        std::cout << "  [!] No playlist selected.\n";
        return;
    }
    playlistManager.getPlaylistConst(selectedPlaylistIndex).display();
}

void Application::shufflePlaylist() {
    if (selectedPlaylistIndex < 0) {
        std::cout << "  [!] No playlist selected.\n";
        return;
    }
    Playlist& pl = playlistManager.getPlaylist(selectedPlaylistIndex);
    if (pl.getTrackCount() <= 1) {
        std::cout << "  [!] Need at least 2 tracks to shuffle.\n";
        return;
    }
    std::cout << "  Shuffling \"" << pl.getName() << "\"...\n";
    pl.shuffle();
    std::cout << "  [OK] Playlist shuffled! New order:\n";
    pl.display();
    std::cout << "  (Shuffling uses std::shuffle with std::mt19937 — Fisher-Yates algorithm)\n";
    std::cout << "  (Track membership is preserved; only the order changes)\n";
}

void Application::sortPlaylist() {
    if (selectedPlaylistIndex < 0) {
        std::cout << "  [!] No playlist selected.\n";
        return;
    }
    std::cout << "  Sort by: 1) Title  2) Artist  3) Duration\n";
    int choice = readInt("  Enter your choice: ");
    Playlist& pl = playlistManager.getPlaylist(selectedPlaylistIndex);
    switch (choice) {
        case 1: pl.sortByTitle();    std::cout << "  [OK] Sorted by title.\n"; break;
        case 2: pl.sortByArtist();   std::cout << "  [OK] Sorted by artist.\n"; break;
        case 3: pl.sortByDuration(); std::cout << "  [OK] Sorted by duration.\n"; break;
        default: std::cout << "  [!] Invalid choice.\n"; return;
    }
    pl.display();
}

void Application::renamePlaylist() {
    if (selectedPlaylistIndex < 0) {
        std::cout << "  [!] No playlist selected.\n";
        return;
    }
    std::string newName = readString("  Enter new name for the playlist: ");
    if (playlistManager.renamePlaylist(selectedPlaylistIndex, newName)) {
        std::cout << "  [OK] Playlist renamed to \"" << newName << "\".\n";
    } else {
        std::cout << "  [!] Could not rename. Name may already exist or is empty.\n";
    }
}

void Application::deletePlaylist() {
    if (playlistManager.getPlaylistCount() == 0) {
        std::cout << "  [!] No playlists to delete.\n";
        return;
    }
    playlistManager.displayAll();
    int idx = readInt("  Enter playlist number to delete: ");
    idx -= 1;
    if (idx < 0 || idx >= playlistManager.getPlaylistCount()) {
        std::cout << "  [!] Invalid playlist number.\n";
        return;
    }
    std::string name = playlistManager.getPlaylistConst(idx).getName();
    if (playlistManager.deletePlaylist(idx)) {
        std::cout << "  [OK] Playlist \"" << name << "\" deleted.\n";
        // Reset selection if the deleted playlist was selected
        if (selectedPlaylistIndex == idx) {
            selectedPlaylistIndex = -1;
        } else if (selectedPlaylistIndex > idx) {
            --selectedPlaylistIndex;
        }
    }
}

void Application::savePlaylist() {
    if (selectedPlaylistIndex < 0) {
        std::cout << "  [!] No playlist selected. Select one first.\n";
        pressEnterToContinue();
        return;
    }
    const Playlist& pl = playlistManager.getPlaylistConst(selectedPlaylistIndex);
    std::string filename = PlaylistStorage::getDefaultDirectory()
                         + PlaylistStorage::generateFilename(pl.getName());

    std::cout << "  Saving playlist \"" << pl.getName()
              << "\" to " << filename << "...\n";

    if (storage.savePlaylist(pl, filename)) {
        std::cout << "  [OK] Playlist saved successfully to: " << filename << "\n";
    } else {
        std::cout << "  [!] Failed to save playlist.\n";
    }
    pressEnterToContinue();
}

void Application::loadPlaylist() {
    std::string filename = readString("  Enter filename to load (e.g., data/my_playlist.csv): ");

    Playlist loaded;
    std::cout << "  Loading playlist from " << filename << "...\n";

    if (storage.loadPlaylist(filename, loaded)) {
        // Check if a playlist with this name already exists
        if (playlistManager.nameExists(loaded.getName())) {
            std::cout << "  [!] A playlist named \"" << loaded.getName()
                      << "\" already exists.\n";
            std::cout << "  The loaded playlist will be renamed with a suffix.\n";
            std::string newName = loaded.getName() + " (loaded)";
            loaded.setName(newName);
        }
        playlistManager.addPlaylist(loaded);
        selectedPlaylistIndex = playlistManager.getPlaylistCount() - 1;
        std::cout << "  [OK] Playlist \"" << loaded.getName()
                  << "\" loaded with " << loaded.getTrackCount() << " tracks.\n";
        loaded.display();
    } else {
        std::cout << "  [!] Failed to load playlist from: " << filename << "\n";
    }
    pressEnterToContinue();
}

// ══════════════════════════════════════════════════════════════════════
// Player operations
// ══════════════════════════════════════════════════════════════════════

void Application::loadPlaylistIntoPlayer() {
    if (selectedPlaylistIndex < 0) {
        std::cout << "  [!] No playlist selected. Select one first.\n";
        return;
    }
    const Playlist& pl = playlistManager.getPlaylistConst(selectedPlaylistIndex);
    player.loadPlaylist(&pl);
    std::cout << "  [OK] Playlist \"" << pl.getName()
              << "\" loaded into the player.\n";
}

// ══════════════════════════════════════════════════════════════════════
// Input helpers
// ══════════════════════════════════════════════════════════════════════

int Application::readInt(const std::string& prompt) const {
    std::cout << prompt;
    std::string input;
    std::getline(std::cin, input);

    // Handle empty input
    if (input.empty()) return -1;

    try {
        size_t pos;
        int value = std::stoi(input, &pos);
        // Ensure the entire string was consumed (no trailing chars)
        if (pos != input.length()) return -1;
        return value;
    } catch (const std::exception&) {
        return -1;
    }
}

std::string Application::readString(const std::string& prompt) const {
    std::cout << prompt;
    std::string input;
    std::getline(std::cin, input);
    // Trim whitespace
    size_t start = input.find_first_not_of(" \t");
    if (start == std::string::npos) return "";
    size_t end = input.find_last_not_of(" \t");
    return input.substr(start, end - start + 1);
}

void Application::pressEnterToContinue() const {
    std::cout << "\n  Press Enter to continue...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

void Application::clearScreen() const {
    // Best-effort clear using ANSI escape on Windows 10+
    // Falls back gracefully if not supported
    #ifdef _WIN32
        std::system("cls");
    #else
        std::system("clear");
    #endif
}

void Application::separator() const {
    std::cout << "  " << std::string(60, '-') << "\n";
}

void Application::displaySearchResults(const std::vector<Song>& results) const {
    if (results.empty()) {
        std::cout << "  [!] No matching songs found.\n";
        return;
    }
    std::cout << "\n  Found " << results.size() << " result(s):\n";
    std::cout << "  " << std::left
              << std::setw(5)  << "ID"
              << std::setw(30) << "Title"
              << std::setw(22) << "Artist"
              << std::setw(14) << "Genre"
              << std::right << std::setw(6) << "Time"
              << "\n  " << std::string(77, '-') << "\n";
    for (const auto& s : results) {
        s.display();
    }
}
