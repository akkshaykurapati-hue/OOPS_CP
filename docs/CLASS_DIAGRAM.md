# Class Diagram — Smart Playlist Synthesizer

This diagram represents the actual class relationships in the implemented codebase.

## Complete Class Diagram

```mermaid
classDiagram
    class Song {
        -int id
        -string title
        -string artist
        -string genre
        -int durationSeconds
        +Song()
        +Song(id, title, artist, genre, durationSeconds)
        +getId() int
        +getTitle() string
        +getArtist() string
        +getGenre() string
        +getDurationSeconds() int
        +getFormattedDuration() string
        +display() void
        +displayDetailed() void
        +validate(id, title, artist, genre, dur)$ void
        +operator==(other) bool
        +operator!=(other) bool
    }

    class MusicLibrary {
        -vector~Song~ songs
        -unordered_set~int~ songIds
        +MusicLibrary()
        +addSong(song) bool
        +removeSong(id) bool
        +containsSong(id) bool
        +getSongCount() int
        +findById(id) Song*
        +searchByTitle(query) vector~Song~
        +searchByArtist(query) vector~Song~
        +searchByGenre(query) vector~Song~
        +displayAll() void
        +getAllSongs() vector~Song~
        +loadSampleData() void
        -containsIgnoreCase(haystack, needle)$ bool
    }

    class Playlist {
        -string name
        -vector~Song~ tracks
        -unordered_set~int~ trackIds
        +Playlist()
        +Playlist(name)
        +getName() string
        +getTrackCount() int
        +isEmpty() bool
        +setName(newName) void
        +addSong(song) bool
        +removeSong(id) bool
        +containsSong(id) bool
        +getTotalDuration() int
        +shuffle() void
        +shuffle(seed) void
        +sortByTitle() void
        +sortByArtist() void
        +sortByDuration() void
        +findById(id) Song*
        +searchByTitle(query) vector~Song~
        +display() void
        +getTracks() vector~Song~
        +getTrackAt(index) Song
        +rebuildFromTracks(name, tracks) void
    }

    class PlaylistManager {
        -vector~Playlist~ playlists
        +PlaylistManager()
        +createPlaylist(name) bool
        +deletePlaylist(index) bool
        +renamePlaylist(index, newName) bool
        +getPlaylist(index) Playlist
        +getPlaylistConst(index) Playlist
        +getPlaylistCount() int
        +nameExists(name) bool
        +findByName(name) int
        +displayAll() void
        +addPlaylist(playlist) void
    }

    class Playable {
        <<abstract>>
        +~Playable()
        +play()* void
        +pause()* void
        +resume()* void
        +next()* void
        +previous()* void
        +showStatus()* void
    }

    class MusicPlayer {
        -Playlist* currentPlaylist
        -int currentTrackIndex
        -State state
        +MusicPlayer()
        +loadPlaylist(playlist) void
        +hasPlaylist() bool
        +play() void
        +pause() void
        +resume() void
        +next() void
        +previous() void
        +showStatus() void
        +getState() State
        +getStateString() string
        +getCurrentSong() Song*
    }

    class PlaylistStorage {
        +PlaylistStorage()
        +savePlaylist(playlist, filepath) bool
        +loadPlaylist(filepath, outPlaylist) bool
        +getDefaultDirectory()$ string
        +generateFilename(name)$ string
        -parseSongLine(line, outSong) bool
        -trim(str)$ string
    }

    class Application {
        -MusicLibrary library
        -PlaylistManager playlistManager
        -MusicPlayer player
        -PlaylistStorage storage
        -bool running
        -int selectedPlaylistIndex
        +Application()
        +run() void
        -showMainMenu() void
        -handleMainMenu() void
        -handleLibraryMenu() void
        -handlePlaylistMenu() void
        -handlePlayerMenu() void
        -handleSearchMenu() void
        -addSongToLibrary() void
        -addSongToPlaylist() void
        -shufflePlaylist() void
        -savePlaylist() void
        -loadPlaylist() void
        -readInt(prompt) int
        -readString(prompt) string
    }

    %% Relationships
    Playable <|-- MusicPlayer : Inheritance
    Application *-- MusicLibrary : Composition
    Application *-- PlaylistManager : Composition
    Application *-- MusicPlayer : Composition
    Application *-- PlaylistStorage : Composition
    PlaylistManager *-- Playlist : Composition
    MusicLibrary *-- Song : Composition
    Playlist *-- Song : Composition
    MusicPlayer --> Playlist : Association
```

## Relationship Explanations

| Relationship | Type | Explanation |
|---|---|---|
| `Playable` ← `MusicPlayer` | **Inheritance** | MusicPlayer IS-A Playable; overrides pure virtual methods |
| `Application` → `MusicLibrary` | **Composition** | Application owns and manages the library's lifetime |
| `Application` → `PlaylistManager` | **Composition** | Application owns and manages all playlists |
| `Application` → `MusicPlayer` | **Composition** | Application owns the player instance |
| `Application` → `PlaylistStorage` | **Composition** | Application owns the storage handler |
| `PlaylistManager` → `Playlist` | **Composition** | Manager contains zero or more Playlist objects |
| `MusicLibrary` → `Song` | **Composition** | Library contains zero or more Song objects |
| `Playlist` → `Song` | **Composition** | Playlist contains zero or more Song objects (copies) |
| `MusicPlayer` → `Playlist` | **Association** | Player references a Playlist (non-owning pointer) |

## Key Design Decisions

1. **Songs are stored by value** in both the library and playlists. This means playlists have their own copies, and changes to the library don't unexpectedly affect playlists.

2. **MusicPlayer uses a non-owning pointer** (`const Playlist*`) to reference the active playlist. The playlist's lifetime is managed by `PlaylistManager`, and the player must not outlive the playlist it references.

3. **Playable is a genuine abstraction** — it defines what any "playable" entity must support, allowing future extension (e.g., a `RadioPlayer` could also implement `Playable`).
