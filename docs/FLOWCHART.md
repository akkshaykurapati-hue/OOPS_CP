# Flowcharts — Smart Playlist Synthesizer

All flowcharts represent the actual execution paths in the implemented codebase.

---

## 1. Overall Application Execution

```mermaid
flowchart TD
    A[Program starts: main.cpp] --> B[Create Application object]
    B --> C[Constructor loads sample data]
    C --> D[Display banner]
    D --> E[Show Main Menu]
    E --> F{Read user input}
    F -->|1| G[Library Menu]
    F -->|2| H[Playlist Menu]
    F -->|3| I[Player Menu]
    F -->|4| J[Search & Sort Menu]
    F -->|5| K[Save Playlist]
    F -->|6| L[Load Playlist]
    F -->|0| M[Exit - Goodbye]
    F -->|Invalid| N[Show error message]
    G --> E
    H --> E
    I --> E
    J --> E
    K --> E
    L --> E
    N --> E
```

---

## 2. Adding a Song to a Playlist

```mermaid
flowchart TD
    A[User selects 'Add Song to Playlist'] --> B{Is a playlist selected?}
    B -->|No| C[Show error: No playlist selected]
    B -->|Yes| D[Display music library]
    D --> E[User enters song ID]
    E --> F{Does song exist in library?}
    F -->|No| G[Show error: Song not found]
    F -->|Yes| H[Call playlist.addSong]
    H --> I{Check unordered_set for ID}
    I -->|ID exists| J[Return false - DUPLICATE]
    I -->|ID not found| K[Insert ID into unordered_set]
    K --> L[Append Song to vector]
    L --> M[Return true - SUCCESS]
    J --> N[Show: DUPLICATE DETECTED]
    M --> O[Show: Song added successfully]
```

---

## 3. Duplicate Detection Logic

```mermaid
flowchart TD
    A["Playlist::addSong(song) called"] --> B["trackIds.count(song.getId())"]
    B --> C{Count > 0?}
    C -->|Yes - Duplicate| D[Return false]
    D --> E[Vector unchanged]
    D --> F[Set unchanged]
    C -->|No - Unique| G["trackIds.insert(song.getId())"]
    G --> H["tracks.push_back(song)"]
    H --> I[Return true]
    I --> J[Both structures updated consistently]

    style D fill:#ff6666
    style I fill:#66ff66
```

---

## 4. Playlist Shuffling

```mermaid
flowchart TD
    A[User selects Shuffle] --> B{Is a playlist selected?}
    B -->|No| C[Show error]
    B -->|Yes| D{Track count <= 1?}
    D -->|Yes| E[Nothing to shuffle - return]
    D -->|No| F[Generate seed from system clock]
    F --> G["Create std::mt19937 RNG with seed"]
    G --> H["std::shuffle(tracks.begin, tracks.end, rng)"]
    H --> I[Vector order is randomized]
    I --> J[unordered_set is NOT modified]
    J --> K[Track count is preserved]
    K --> L[All track IDs are preserved]
    L --> M[Display new order]
```

---

## 5. Playback Navigation

```mermaid
flowchart TD
    A[Player Menu] --> B{User choice}
    B -->|Load| C[Load playlist pointer into player]
    C --> D[Reset index to -1, state to STOPPED]
    B -->|Play| E{Playlist loaded and non-empty?}
    E -->|No| F[Show error]
    E -->|Yes| G[Set index = 0, state = PLAYING]
    G --> H[Display current track info]
    B -->|Pause| I{State == PLAYING?}
    I -->|No| J[Show: Nothing playing]
    I -->|Yes| K[Set state = PAUSED]
    B -->|Resume| L{State == PAUSED?}
    L -->|No| M[Show: Not paused]
    L -->|Yes| N[Set state = PLAYING]
    B -->|Next| O{At last track?}
    O -->|Yes| P[Show: Already at last track]
    O -->|No| Q[Increment index]
    Q --> R[Set state = PLAYING, display track]
    B -->|Previous| S{At first track?}
    S -->|Yes| T[Show: Already at first track]
    S -->|No| U[Decrement index]
    U --> V[Set state = PLAYING, display track]
```

---

## 6. Saving and Loading Playlists

### Save Flow

```mermaid
flowchart TD
    A[User selects Save] --> B{Playlist selected?}
    B -->|No| C[Show error]
    B -->|Yes| D[Generate filename from playlist name]
    D --> E["Open ofstream to data/filename.csv"]
    E --> F{File opened?}
    F -->|No| G[Show error: Cannot write]
    F -->|Yes| H["Write PLAYLIST:name header"]
    H --> I[For each track in vector]
    I --> J["Write: id,title,artist,genre,duration"]
    J --> K{More tracks?}
    K -->|Yes| I
    K -->|No| L[Close file]
    L --> M{Write successful?}
    M -->|No| N[Show error]
    M -->|Yes| O[Show: Saved successfully]
```

### Load Flow

```mermaid
flowchart TD
    A[User enters filename] --> B["Open ifstream"]
    B --> C{File exists?}
    C -->|No| D[Show error: File not found]
    C -->|Yes| E[Read first line]
    E --> F{Starts with 'PLAYLIST:'?}
    F -->|Yes| G[Extract playlist name]
    F -->|No| H[Use default name]
    G --> I[Read subsequent lines]
    H --> I
    I --> J[Parse CSV: id,title,artist,genre,duration]
    J --> K{Parse successful?}
    K -->|No| L[Log warning, skip record]
    K -->|Yes| M["Validate via Song::validate()"]
    M --> N{Valid?}
    N -->|No| L
    N -->|Yes| O[Add to temporary vector]
    O --> P{More lines?}
    L --> P
    P -->|Yes| I
    P -->|No| Q["Call rebuildFromTracks()"]
    Q --> R[Clear existing tracks and IDs]
    R --> S["For each loaded song: addSong()"]
    S --> T[Rebuild unordered_set]
    T --> U[Add playlist to manager]
    U --> V[Show: Loaded successfully]
```
