# Project Report — Smart Playlist Synthesizer

**Course**: Object-Oriented Programming (OOP) Laboratory  
**Level**: Second Year B.Tech (Computer Engineering / Information Technology)  
**Academic Year**: 2025–2026  
**Project No.**: 28  
**Project Title**: Smart Playlist Synthesizer  

---

## Team Members

| Sr. No. | Student Name | Role | Responsibilities |
|---------|--------------|------|------------------|
| 1 | Akshay Kurapati | Lead / Developer | Project Architecture, Core OOP Design, Testing |
| 2 | Atharv Mane | Developer | Music Library & Playlist Data Structures |
| 3 | Manas Chaudhari | Developer | Playback Simulation (`Playable`, `MusicPlayer`) |
| 4 | Piyush Lasurkar | Developer | File Persistence & CSV Parser (`PlaylistStorage`) |
| 5 | Naman Kumar Rai | QA / Testing | Test Suite Execution, Edge Cases Validation |
| 6 | RupeshKumar Nirankari | Documentation | Report Generation, Diagrams, Viva Prep |

---

## 1. Executive Summary

The **Smart Playlist Synthesizer** is a modular C++ console application designed to fulfill Problem Statement No. 28 of the Object-Oriented Programming course. The objective was to engineer a media management utility that strictly prevents duplicate track entries using specialized set structures and enables dynamic playlist synthesis and randomized track shuffling by reordering underlying indexed sequence containers.

Built entirely in modern C++ (C++17 standard) without external third-party dependencies, the system applies core Object-Oriented principles (Encapsulation, Abstraction, Inheritance, and Runtime Polymorphism) alongside modern C++ Standard Template Library (STL) components. It includes an interactive user interface, an end-to-end file persistence mechanism, and a test suite of 138 unit tests verifying data integrity, duplicate rejections, shuffling properties, and error handling.

---

## 2. Problem Statement & Scope

### 2.1 Problem Statement (Project No. 28)
> *"Code a media tracking application preventing duplicate entries using specialized Sets, allowing dynamic shuffling mechanics by altering underlying indexed sequence configurations."*

### 2.2 Prescribed Scope & Objectives
1. **Duplicate Prevention via Sets**: Implement track tracking using a hash-based set (`std::unordered_set<int>`) to filter song additions in $O(1)$ expected time complexity.
2. **Dynamic Shuffling via Sequential Containers**: Maintain songs sequentially in an indexed list (`std::vector<Song>`) to enable random-access indexing, sorting, and Fisher-Yates-based shuffling via `std::shuffle` and Mersenne Twister engine (`std::mt19937`).
3. **Decoupled Architecture**: Isolate the business data domain (`Song`, `Playlist`, `MusicLibrary`) from playback and hardware simulation modules (`Playable`, `MusicPlayer`), managed via an application coordinator (`Application`).
4. **Persistence & Robustness**: Support exporting and importing playlists to/from CSV storage with validation against corrupted entries.

---

## 3. System Architecture & OOP Design

The system adheres to standard Object-Oriented and SOLID design principles.

```
                    +-------------------+
                    |    Application    |
                    +---------+---------+
                              |
       +----------------------+----------------------+
       |                      |                      |
       v                      v                      v
+--------------+    +-------------------+    +---------------+
| MusicLibrary |    |  PlaylistManager  |    |   Playable    | (Interface)
+-------+------+    +---------+---------+    +-------^-------+
        |                     |                      |
        | 1..*                | 1..*                 | inherits
        v                     v                      |
     +------+              +----------+      +-------+-------+
     | Song |              | Playlist |      |  MusicPlayer  |
     +------+              +----+-----+      +---------------+
                                |
                                v
                       +-----------------+
                       | PlaylistStorage |
                       +-----------------+
```

### 3.1 Class Responsibilities

1. **`Song`**:
   - Represents an immutable music track entity.
   - Encapsulates ID, title, artist, duration (seconds), and genre.
   - Enforces validation: ID > 0, duration > 0, non-empty text strings.

2. **`MusicLibrary`**:
   - Manages the master repository of available songs.
   - Provides $O(1)$ fast lookup by ID via `std::unordered_map<int, Song>`.
   - Offers filtering and sorting by title, artist, or genre.

3. **`Playlist`**:
   - Combines a `std::vector<Song>` (for ordered playback, indexed access, and shuffling) with a `std::unordered_set<int>` (for instant duplicate checking).
   - Guarantees synchronization: every track in the vector has its ID tracked in the set.
   - Provides operations: `addSong()`, `removeSong()`, `shuffle()`, `sort()`.

4. **`PlaylistManager`**:
   - Manages a collection of playlists (`std::vector<Playlist>`).
   - Supports creating, selecting, renaming, and deleting playlists.

5. **`Playable` (Pure Abstract Class / Interface)**:
   - Defines the playback contract: `play()`, `pause()`, `resume()`, `stop()`, `next()`, `previous()`.
   - Contains pure virtual methods (`= 0`) and a virtual destructor.

6. **`MusicPlayer`**:
   - Implements `Playable`.
   - Encapsulates playback state (`STOPPED`, `PLAYING`, `PAUSED`) and tracks current index.
   - Provides bounds checking and wrap-around / linear playback logic.

7. **`PlaylistStorage`**:
   - Handles file I/O operations using standard streams (`std::ifstream`, `std::ofstream`).
   - Serializes playlists into comma-separated values (CSV) and parses files back into memory.

8. **`Application`**:
   - Acts as the presentation layer and facade for the entire system.
   - Renders interactive CLI menus and validates user input.

---

## 4. Key Algorithms & Data Structures

| Operation | Container / Algorithm | Time Complexity | Purpose |
|-----------|----------------------|-----------------|---------|
| Duplicate Check | `std::unordered_set<int>` | $O(1)$ avg, $O(N)$ worst | Check if song ID is already present before adding |
| Ordered Storage | `std::vector<Song>` | $O(1)$ access, $O(1)$ append | Indexed sequence configuration for sequential traversal |
| Master Lookup | `std::unordered_map<int, Song>` | $O(1)$ avg | Retrieve complete song metadata by integer ID |
| Track Shuffling | `std::shuffle` + `std::mt19937` | $O(N)$ | Cryptographically uniform permutation of track sequence |
| Track Sorting | `std::sort` with custom lambdas | $O(N \log N)$ | Reorder by title, artist, or duration |
| Linear Search | `std::find_if` | $O(N)$ | Case-insensitive substring matching for query filters |

---

## 5. Verification & Testing

The test suite is structured into two standalone test executables built on an explicit assertions framework without heavy external testing libraries:

### 5.1 Test Breakdown

1. **`playlist_tests.exe` (108 Assertions)**:
   - Song constructor validation (positive IDs, non-empty fields, positive durations).
   - Duplicate prevention verification (multiple additions of identical IDs rejected).
   - Sequence maintenance upon removals and additions.
   - Deterministic and non-deterministic shuffling verifications.
   - Sorting correctness across title, artist, and duration criteria.
   - MusicPlayer state machine transitions (STOPPED -> PLAYING -> PAUSED -> RESUMED).
   - Player boundaries (first track previous, last track next).

2. **`storage_tests.exe` (30 Assertions)**:
   - CSV serialization to temporary files.
   - Correct parsing of CSV records back into `Playlist` objects.
   - Duplicate rejection during corrupted file ingestion.
   - Preservation of track order across save/load cycles.

### 5.2 Verification Summary
- **Total Tests Executed**: 138
- **Tests Passed**: 138
- **Tests Failed**: 0
- **Pass Rate**: 100%

---

## 6. Conclusion & Learning Outcomes

Through the development of the Smart Playlist Synthesizer, all academic and technical project goals were achieved:
- Complete mastery over object encapsulation, member protection, and class contracts.
- Practical experience designing abstract interfaces (`Playable`) and realizing runtime polymorphism.
- Practical understanding of STL containers, specifically balancing the complementary strengths of hash sets (`unordered_set`) and dynamic arrays (`vector`).
- Rigorous unit testing practices ensuring bug-free delivery and stability under edge-case conditions.
