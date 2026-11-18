# OOP Concepts — Smart Playlist Synthesizer

This document maps every OOP concept to its actual implementation in the codebase, with code excerpts, explanations, and likely professor questions.

---

## 1. Encapsulation

### Definition
Encapsulation bundles data (attributes) and the methods that operate on that data into a single unit (class), restricting direct access to the internal state through access specifiers (`private`, `public`).

### Where it is used
**Every class** in this project uses encapsulation. The most important example is the `Playlist` class.

### Source file
`include/Playlist.h`, `src/Playlist.cpp`

### Code excerpt
```cpp
class Playlist {
private:
    std::string name;                   // Hidden from external code
    std::vector<Song> tracks;           // Hidden — cannot be modified directly
    std::unordered_set<int> trackIds;   // Hidden — ensures consistency with tracks

public:
    bool addSong(const Song& song);     // Controlled access point
    bool removeSong(int id);            // Ensures both structures stay in sync
    bool containsSong(int id) const;    // Read-only access
    int getTrackCount() const;
};
```

### Why this matters
If `tracks` and `trackIds` were public, external code could add a song to the vector without updating the set, breaking duplicate prevention. By making them private and routing all modifications through `addSong()` and `removeSong()`, we guarantee both data structures always remain consistent.

### Practical example
When a user adds song ID 101 to a playlist:
1. `addSong()` checks `trackIds` for ID 101
2. If not found, inserts 101 into `trackIds` AND appends the song to `tracks`
3. External code cannot skip the set check

### Likely professor question
**Q:** Why are `tracks` and `trackIds` private? What would happen if they were public?
**A:** If they were public, external code could modify `tracks` without updating `trackIds`, leading to inconsistencies — a song could exist in the vector but not in the set, or vice versa. Duplicate prevention would break.

---

## 2. Abstraction

### Definition
Abstraction exposes only essential features while hiding implementation details. Users of a class interact with its public interface without knowing how methods work internally.

### Where it is used
`Playlist::addSong()`, `Playlist::shuffle()`, `PlaylistStorage::savePlaylist()`

### Source file
`src/Playlist.cpp`

### Code excerpt
```cpp
// The caller simply writes:
playlist.addSong(mySong);
playlist.shuffle();

// They don't need to know that addSong():
//   1. Checks an unordered_set
//   2. Inserts into the set
//   3. Appends to a vector
// Or that shuffle() uses:
//   std::mt19937 with a time-based seed
//   std::shuffle on the internal vector
```

### Why this matters
The `Application` class calls `playlist.shuffle()` without knowing about `std::mt19937` or `std::shuffle`. If we later changed the shuffling algorithm (e.g., to a custom implementation), the Application code would not need to change.

### Likely professor question
**Q:** Give an example of abstraction in your project.
**A:** `Playlist::shuffle()` abstracts the shuffling mechanism. The caller doesn't know it uses `std::mt19937` and `std::shuffle` internally — they just call `shuffle()` and the playlist order changes.

---

## 3. Inheritance

### Definition
Inheritance allows a class (derived) to acquire properties and behaviors from another class (base), promoting code reuse and establishing an IS-A relationship.

### Where it is used
`MusicPlayer` inherits from abstract class `Playable`.

### Source files
`include/Playable.h`, `include/MusicPlayer.h`, `src/MusicPlayer.cpp`

### Code excerpt
```cpp
// Base class (abstract interface)
class Playable {
public:
    virtual ~Playable() {}
    virtual void play() = 0;      // Pure virtual
    virtual void pause() = 0;     // Pure virtual
    virtual void resume() = 0;    // Pure virtual
    virtual void next() = 0;      // Pure virtual
    virtual void previous() = 0;  // Pure virtual
    virtual void showStatus() const = 0;
};

// Derived class
class MusicPlayer : public Playable {
public:
    void play() override;      // Implements the pure virtual
    void pause() override;
    void resume() override;
    void next() override;
    void previous() override;
    void showStatus() const override;
};
```

### Why this design
`Playable` defines **what** a player must do (contract). `MusicPlayer` defines **how** it does it (implementation). This separation allows future extension — a `RadioPlayer` or `PodcastPlayer` could also implement `Playable`.

### Likely professor question
**Q:** Why did you create a separate `Playable` class? Why not put everything in `MusicPlayer`?
**A:** `Playable` serves as an abstract interface. It defines the contract that any playback component must follow. This enables polymorphism — code can work with a `Playable*` pointer without knowing the concrete type. It also supports future extensibility.

---

## 4. Polymorphism

### Definition
Polymorphism allows objects of different types to be treated through a common interface. In C++, runtime polymorphism is achieved through virtual functions and base-class pointers/references.

### Where it is used
`Application::handlePlayerMenu()` in `src/Application.cpp`

### Code excerpt
```cpp
void Application::handlePlayerMenu() {
    // ...
    // Base-class pointer to derived object — POLYMORPHISM
    Playable* playable = &player;   // player is MusicPlayer

    switch (choice) {
        case 2: playable->play();      break;  // Calls MusicPlayer::play()
        case 3: playable->pause();     break;  // Calls MusicPlayer::pause()
        case 4: playable->resume();    break;  // Calls MusicPlayer::resume()
        case 5: playable->next();      break;  // Calls MusicPlayer::next()
        case 6: playable->previous();  break;  // Calls MusicPlayer::previous()
        case 7: playable->showStatus();break;  // Calls MusicPlayer::showStatus()
    }
}
```

### How it works
Even though `playable` is declared as `Playable*` (base class), calling `playable->play()` invokes `MusicPlayer::play()` (derived class) because `play()` is a virtual function. The correct method is resolved at runtime based on the actual object type.

### Likely professor question
**Q:** Where exactly is polymorphism demonstrated? Show the base-class pointer call.
**A:** In `Application::handlePlayerMenu()`, the `Playable* playable = &player;` line creates a base-class pointer to a `MusicPlayer` object. When `playable->play()` is called, the virtual function mechanism resolves to `MusicPlayer::play()` at runtime.

---

## 5. Composition

### Definition
Composition is a strong HAS-A relationship where one class contains objects of another class. The contained objects' lifetimes are managed by the container.

### Where it is used
Multiple places:
- `Application` HAS-A `MusicLibrary`, `PlaylistManager`, `MusicPlayer`, `PlaylistStorage`
- `PlaylistManager` HAS-A `vector<Playlist>`
- `Playlist` HAS-A `vector<Song>` and `unordered_set<int>`
- `MusicLibrary` HAS-A `vector<Song>` and `unordered_set<int>`

### Source file
`include/Application.h`

### Code excerpt
```cpp
class Application {
private:
    MusicLibrary library;           // HAS-A (composition)
    PlaylistManager playlistManager; // HAS-A (composition)
    MusicPlayer player;             // HAS-A (composition)
    PlaylistStorage storage;        // HAS-A (composition)
};
```

### vs. Association
`MusicPlayer` uses **association** (not composition) with `Playlist`:
```cpp
class MusicPlayer {
private:
    const Playlist* currentPlaylist;  // Non-owning pointer (association)
};
```
The player references a playlist but doesn't own it. The playlist is owned by `PlaylistManager`.

### Likely professor question
**Q:** What is the difference between composition and association in your project?
**A:** `Application` uses composition — it directly contains `MusicLibrary`, `PlaylistManager`, etc. as member variables. Their lifetimes are tied to `Application`. `MusicPlayer` uses association — it holds a non-owning pointer to a `Playlist` owned by `PlaylistManager`. The player doesn't control the playlist's lifetime.

---

## 6. Constructors and Initialization

### Definition
Constructors are special methods called when an object is created. They initialize the object's state.

### Where it is used
All classes have constructors. `Song` demonstrates parameterized constructors with validation.

### Source file
`src/Song.cpp`

### Code excerpt
```cpp
Song::Song(int id, const std::string& title, const std::string& artist,
           const std::string& genre, int durationSeconds)
    : id(id), title(title), artist(artist), genre(genre),
      durationSeconds(durationSeconds)
{
    validate(id, title, artist, genre, durationSeconds);
}
```

### Key points
- **Member initializer list** (`: id(id), title(title), ...`) initializes members directly, avoiding default construction + assignment.
- **Validation in constructor** ensures no invalid `Song` object can exist.
- **Default constructor** (`Song()`) exists for STL container operations.

### Likely professor question
**Q:** Why use a member initializer list instead of assignment in the constructor body?
**A:** The initializer list directly constructs each member with the given value, avoiding unnecessary default construction followed by assignment. For `std::string` members, this is more efficient.

---

## 7. Access Specifiers

### Definition
Access specifiers (`private`, `public`, `protected`) control visibility and accessibility of class members.

### Usage in the project

| Specifier | Where | Purpose |
|-----------|-------|---------|
| `private` | All data members in Song, Playlist, MusicLibrary | Prevent uncontrolled modification |
| `public` | Getters, display methods, core operations | Provide controlled interface |
| `protected` | Not used | Not needed — no intermediate inheritance layers |
| `private` (Application) | Menu handlers, input helpers | Internal implementation details |

### Why `protected` is not used
`protected` would be appropriate if intermediate derived classes needed access to base-class members. Our inheritance hierarchy (`Playable` → `MusicPlayer`) uses pure virtual functions that are always public. There's no meaningful case for `protected` here.

### Likely professor question
**Q:** Why didn't you use `protected` anywhere?
**A:** `protected` is useful when derived classes need direct access to base-class data. In our project, `Playable` is an abstract interface with only pure virtual functions (always public). `MusicPlayer` has its own private data. There's no inheritance scenario requiring `protected` access.

---

## 8. Const Correctness

### Definition
`const` correctness means marking methods and parameters as `const` when they don't modify the object's state, preventing accidental modifications.

### Where it is used
Throughout the codebase — getters, search methods, display methods, and parameter passing.

### Code excerpt
```cpp
// Const methods — do not modify the Playlist object
int Playlist::getTrackCount() const { return static_cast<int>(tracks.size()); }
bool Playlist::isEmpty() const { return tracks.empty(); }
bool Playlist::containsSong(int id) const { return trackIds.count(id) > 0; }
void Playlist::display() const { /* only reads data */ }

// Const reference parameters — do not copy the Song
bool Playlist::addSong(const Song& song) { /* ... */ }

// Const return — read-only access to internal vector
const std::vector<Song>& Playlist::getTracks() const { return tracks; }
```

### Why this matters
- `const` methods can be called on `const` references/pointers
- `PlaylistManager::getPlaylistConst(int index) const` returns a `const Playlist&` — the caller cannot modify it
- Prevents bugs where read-only operations accidentally change state

### Likely professor question
**Q:** What happens if you remove `const` from `getTrackCount()`?
**A:** Code that holds a `const Playlist&` reference (like `PlaylistManager::getPlaylistConst()`) would fail to compile when calling `getTrackCount()`, because the compiler cannot guarantee a non-const method won't modify the object.

---

## 9. Exception Handling

### Definition
Exception handling uses `try`, `throw`, and `catch` to handle runtime errors gracefully without crashing the program.

### Where it is used
1. **Song validation** — throws `std::invalid_argument`
2. **File operations** — catches `std::exception` in `PlaylistStorage`
3. **Index bounds** — throws `std::out_of_range` in `getTrackAt()`, `getPlaylist()`
4. **Main function** — top-level `try/catch` in `main()`
5. **Input parsing** — catches `std::stoi` exceptions in `readInt()`

### Source files
`src/Song.cpp`, `src/PlaylistStorage.cpp`, `src/Application.cpp`, `src/main.cpp`

### Code excerpt
```cpp
// Song constructor throws on invalid data
Song::Song(int id, ...) {
    validate(id, title, artist, genre, durationSeconds);
    // validate() throws std::invalid_argument if invalid
}

// Application catches the exception
void Application::addSongToLibrary() {
    try {
        Song newSong(id, title, artist, genre, duration);
        // ... add to library
    } catch (const std::invalid_argument& e) {
        std::cout << "  [ERROR] " << e.what() << "\n";
    }
}

// Top-level safety net in main()
int main() {
    try {
        Application app;
        app.run();
    } catch (const std::exception& e) {
        std::cerr << "[FATAL ERROR] " << e.what() << "\n";
        return 1;
    }
}
```

### Likely professor question
**Q:** Why not use exception handling for all input validation?
**A:** Simple input errors (like non-numeric menu input) are handled with `if` checks and return values — using exceptions for routine expected situations is considered poor practice in C++. Exceptions are reserved for truly exceptional circumstances like invalid object construction and file I/O failures.

---

## 10. STL (Standard Template Library) Usage

### Definition
The STL provides template-based containers, algorithms, and iterators. This project uses several STL components as required by the official project specification.

### Components used

| STL Component | Purpose | Source File |
|--------------|---------|-------------|
| `std::vector<Song>` | Ordered storage of songs in library and playlists | `Playlist.h`, `MusicLibrary.h` |
| `std::unordered_set<int>` | O(1) average-case duplicate ID detection | `Playlist.h`, `MusicLibrary.h` |
| `std::string` | Text storage for titles, artists, genres | `Song.h` |
| `std::shuffle()` | Fisher-Yates shuffle algorithm for randomizing order | `Playlist.cpp` |
| `std::mt19937` | Mersenne Twister pseudo-random number generator | `Playlist.cpp` |
| `std::sort()` | Introsort for ordering tracks by title/artist/duration | `Playlist.cpp` |
| `std::find_if()` | Linear search with a predicate | `MusicLibrary.cpp`, `Playlist.cpp` |
| `std::remove_if()` | Erase-remove idiom for deletion from vector | `MusicLibrary.cpp`, `Playlist.cpp` |
| `std::ifstream` / `std::ofstream` | File input/output streams | `PlaylistStorage.cpp` |
| `std::istringstream` / `std::ostringstream` | String stream operations | `Song.cpp`, `PlaylistStorage.cpp` |

### Why vector + unordered_set together?

| Operation | vector alone | unordered_set alone | Both together |
|-----------|-------------|-------------------|--------------|
| Ordered access | ✅ O(1) by index | ❌ No order | ✅ Vector provides order |
| Duplicate check | ❌ O(n) linear scan | ✅ O(1) average | ✅ Set provides O(1) check |
| Shuffling | ✅ Random access | ❌ No concept of order | ✅ Shuffle the vector |
| Display in order | ✅ Iterate | ❌ No order guarantee | ✅ Iterate the vector |

### Likely professor question
**Q:** Why use both `vector` and `unordered_set`? Couldn't you use just one?
**A:** A vector alone would require O(n) linear search for duplicate detection. An unordered_set alone cannot maintain insertion order or support shuffling. Using both gives us O(1) average duplicate detection AND ordered, shuffleable storage.

---

## Design Decision Summary

| Decision | Chosen Approach | Alternative Considered | Why This Choice |
|----------|----------------|----------------------|-----------------|
| Duplicate prevention | `unordered_set<int>` | `set<int>` | O(1) average vs O(log n) — order not needed for ID lookup |
| Song storage | `vector<Song>` (by value) | `vector<shared_ptr<Song>>` | Simpler, no lifetime issues, songs are small |
| Inheritance | `Playable` → `MusicPlayer` | No inheritance | Meaningful abstraction for playback contract |
| File format | CSV | JSON, binary | Simple, human-readable, sufficient for this project |
| Player-Playlist link | Non-owning pointer | Storing a copy | Avoids duplication, reflects "reads from" relationship |
| Random generation | `std::mt19937` | `rand()` | Better distribution, reproducible with seed |
