# Codebase Walkthrough & Architecture Explanation

This document provides a line-by-line and module-by-module architectural explanation of the **Smart Playlist Synthesizer**. It is intended as an in-depth guide for professors, reviewers, and team members to understand every design decision, data member, and member function in the project.

---

## 1. Directory Structure Overview

```
SmartPlaylistSynthesizer/
├── include/              # Class declarations and interfaces
│   ├── Song.h            # Song entity model
│   ├── MusicLibrary.h    # Master library container & lookup
│   ├── Playlist.h        # Vector + HashSet hybrid collection
│   ├── PlaylistManager.h # Multi-playlist coordinator
│   ├── Playable.h        # Pure abstract interface for audio playback
│   ├── MusicPlayer.h     # Playback state engine implementing Playable
│   ├── PlaylistStorage.h # File persistence & CSV serialization
│   └── Application.h     # Console UI and menu system
├── src/                  # Concrete implementations
│   ├── main.cpp          # Application entry point
│   ├── Song.cpp
│   ├── MusicLibrary.cpp
│   ├── Playlist.cpp
│   ├── PlaylistManager.cpp
│   ├── MusicPlayer.cpp
│   ├── PlaylistStorage.cpp
│   └── Application.cpp
└── tests/                # Unit test suites
    ├── playlist_tests.cpp# 108 tests for data structures & player
    └── storage_tests.cpp # 30 tests for file persistence
```

---

## 2. Component-by-Component Walkthrough

### 2.1 Song (`Song.h`, `Song.cpp`)

#### Purpose
`Song` represents an individual audio track entity with core metadata: identifier, title, artist, genre, and duration in seconds.

#### Data Members
- `int id`: Unique numerical identifier for the song.
- `std::string title`: Title of the song.
- `std::string artist`: Name of the performing artist.
- `int durationSeconds`: Track length in seconds.
- `std::string genre`: Musical genre (e.g., "Rock", "Pop", "Jazz").

#### Key Methods & Design Details
- **Parameterized Constructor**: `Song(int id, const std::string& title, const std::string& artist, int duration, const std::string& genre)`
  - Validates that `id > 0`, `duration > 0`, and strings are non-empty. Throws `std::invalid_argument` on invalid inputs.
- **Getters**: All getters (`getId()`, `getTitle()`, etc.) are declared `const` to guarantee const correctness.
- **`formatDuration() const`**: Formats the integer duration into standard `MM:SS` format using `std::ostringstream` and `std::setw(2)`.
- **Operator Overloads**:
  - `operator==`: Compares songs by unique `id`.
  - `operator!=`: Negation of `==`.
  - `operator<<`: Stream insertion for formatted console output.

---

### 2.2 MusicLibrary (`MusicLibrary.h`, `MusicLibrary.cpp`)

#### Purpose
Acts as the global repository of all available music tracks that can be added to playlists.

#### Data Members
- `std::vector<Song> songs`: Preserves insertion order of all library tracks for sequential listing.
- `std::unordered_map<int, Song> songMap`: Hash table mapping `id -> Song` to allow constant time $O(1)$ lookups.

#### Key Methods
- `bool addSong(const Song& song)`: Checks `songMap` to ensure no two tracks in the library have conflicting IDs. Returns `false` if duplicate ID exists.
- `bool removeSong(int songId)`: Erases from both `songMap` and `songs` vector.
- `const Song* findSongById(int songId) const`: Returns a pointer to the song if found, or `nullptr` if absent. Returning a pointer avoids expensive copies and gracefully indicates absence.
- `std::vector<Song> searchByTitle(const std::string& query) const`: Case-insensitive substring search over the track catalog.
- `std::vector<Song> filterByGenre(const std::string& genre) const`: Filters tracks by genre.
- `void loadSampleTracks()`: Populates the library with default tracks on system startup.

---

### 2.3 Playlist (`Playlist.h`, `Playlist.cpp`)

#### Purpose
The core class answering Problem Statement No. 28. It maintains tracks in an indexed sequential configuration while enforcing duplicate prevention through a specialized hash set.

#### Data Members
- `std::string name`: Name of the playlist.
- `std::vector<Song> songs`: Sequential container (`ArrayList` equivalent in C++) enabling indexed random access, ordering, and shuffling.
- `std::unordered_set<int> trackIds`: Specialized hash set (`HashSet` equivalent in C++) tracking IDs of present songs for $O(1)$ duplicate checks.

#### Key Methods
- `bool addSong(const Song& song)`:
  ```cpp
  if (trackIds.find(song.getId()) != trackIds.end()) {
      return false; // Duplicate rejected!
  }
  trackIds.insert(song.getId());
  songs.push_back(song);
  return true;
  ```
  This is the exact requirement of the project: raw track selections are filtered through a HashSet before appending to the sequence.
- `bool removeSong(int songId)`: Removes the track ID from `trackIds` and removes the `Song` from `songs` via `std::remove_if`.
- `void shuffle()`:
  ```cpp
  std::random_device rd;
  std::mt19937 g(rd());
  std::shuffle(songs.begin(), songs.end(), g);
  ```
  Alters the underlying indexed sequence configuration dynamically using the Mersenne Twister pseudo-random number generator.
- `void sortByTitle()`, `sortByArtist()`, `sortByDuration()`: Uses `std::sort` with custom lambdas to rearrange the vector.
- `int getTotalDuration() const`: Accumulates total playlist run time across all tracks.

---

### 2.4 PlaylistManager (`PlaylistManager.h`, `PlaylistManager.cpp`)

#### Purpose
Manages multiple playlists in the application, maintaining the active playlist context.

#### Data Members
- `std::vector<Playlist> playlists`: List of user-created playlists.
- `int activeIndex`: Index of the currently selected playlist (or `-1` if none selected).

#### Key Methods
- `bool createPlaylist(const std::string& name)`: Ensures playlist names are non-empty and unique.
- `bool selectPlaylist(size_t index)`: Sets `activeIndex` with bounds checking.
- `Playlist* getActivePlaylist()`: Returns a mutable pointer to the active playlist.
- `bool deletePlaylist(size_t index)`: Removes playlist and updates `activeIndex`.

---

### 2.5 Playable Interface & MusicPlayer (`Playable.h`, `MusicPlayer.h`, `MusicPlayer.cpp`)

#### Purpose
Demonstrates pure abstraction, inheritance, and runtime polymorphism while decoupling playback mechanics from data entities.

#### Pure Abstract Interface (`Playable`)
```cpp
class Playable {
public:
    virtual ~Playable() = default;
    virtual void play() = 0;
    virtual void pause() = 0;
    virtual void resume() = 0;
    virtual void stop() = 0;
    virtual void next() = 0;
    virtual void previous() = 0;
    virtual std::string getStatus() const = 0;
};
```

#### Implementation (`MusicPlayer`)
- **State Enum**: `enum class PlayState { STOPPED, PLAYING, PAUSED };`
- **Data Members**:
  - `const Playlist* currentPlaylist`: Pointer to the playlist currently loaded for playback.
  - `int currentIndex`: Zero-based index of the currently playing song in the playlist's vector.
  - `PlayState state`: Current state of the playback engine.
- **Methods**:
  - `loadPlaylist(const Playlist* playlist)`: Attaches a playlist and resets the index to 0.
  - `play()`, `pause()`, `resume()`, `stop()`: Transitions between states with descriptive console notifications.
  - `next()`: Advances index to the next song; supports wrap-around to the first song or boundary termination.
  - `previous()`: Moves to previous song with boundary protection.

---

### 2.6 PlaylistStorage (`PlaylistStorage.h`, `PlaylistStorage.cpp`)

#### Purpose
Handles persistence by saving playlist data to disk and reconstructing playlist objects from CSV files.

#### Format
```csv
id,title,artist,durationSeconds,genre
1,Bohemian Rhapsody,Queen,354,Rock
2,Billie Jean,Michael Jackson,294,Pop
```

#### Key Methods
- `bool saveToFile(const Playlist& playlist, const std::string& filepath)`: Serializes headers and song rows to disk.
- `bool loadFromFile(Playlist& playlist, const std::string& filepath, const MusicLibrary* library)`: Parses records, validates fields, and populates the playlist while enforcing duplicate prevention.

---

### 2.7 Application & main (`Application.h`, `Application.cpp`, `main.cpp`)

#### Purpose
Encapsulates application state, coordinates between modules, and renders console menus with robust input validation.

#### Features
- Safe integer input reading with `std::cin.clear()` and line clearing to prevent infinite loops on non-numeric input.
- Clear error and success messages.
- Pre-loads 10 diverse songs across multiple genres for instant testing.
