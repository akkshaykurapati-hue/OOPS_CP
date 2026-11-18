# Demonstration Guide — Smart Playlist Synthesizer

**Audience**: Course Professors, External Examiners, Review Committee  
**Target Duration**: 5 to 7 Minutes  
**Demonstrators**: Akshay Kurapati & Project Team Members  

---

## 1. Preparation Checklist (Before the Presentation)

- [ ] Ensure Windows Terminal or Command Prompt is open at:  
      `c:\Users\Akshay\OneDrive\Desktop\oop cp\SmartPlaylistSynthesizer`
- [ ] Confirm binary compiles cleanly:  
      `g++ -std=c++17 -Wall -Wextra -Iinclude src/*.cpp -o SmartPlaylistSynthesizer.exe`
- [ ] Verify test executables compile and run:  
      `.\playlist_tests.exe` and `.\storage_tests.exe` (138 tests passing)
- [ ] Have sample CSV ready in `data/sample_tracks.csv`

---

## 2. Minute-by-Minute Presentation Script

### Minute 0:00 – 1:00 | Introduction & Problem Statement
1. **Welcome the Examiner**:
   > *"Good morning professors. Today we are presenting Project No. 28: Smart Playlist Synthesizer. Our goal was to build a clean, modular media tracking application in C++ that enforces duplicate track prevention using specialized Hash Sets and allows dynamic shuffling by altering underlying indexed sequence configurations."*
2. **Key Concepts to Mention**:
   - Clean Object-Oriented Architecture (Encapsulation, Abstraction, Inheritance, Polymorphism).
   - STL Containers: `std::vector` (Sequential Array) and `std::unordered_set` (Hash Set).
   - Pure abstract interface `Playable` implemented polymorphically by `MusicPlayer`.

---

### Minute 1:00 – 2:30 | Core Feature 1: Music Library & Duplicate Prevention
1. **Launch the Application**:
   ```powershell
   .\SmartPlaylistSynthesizer.exe
   ```
2. **View Pre-loaded Music Library**:
   - In Main Menu, type `1` (Music Library) -> `1` (Display All Songs).
   - Explain to the professor:
     > *"The library is pre-loaded with 10 diverse songs across multiple genres. Each song has a unique ID, artist, title, genre, and duration."*
   - Return to main menu (`0`).
3. **Create a New Playlist**:
   - Main Menu -> Type `2` (Playlist Management) -> `1` (Create New Playlist).
   - Enter name: `Study Session`.
4. **Demonstrate Duplicate Prevention (Requirement #1)**:
   - In Playlist Management, select option `4` (Add Song to Active Playlist).
   - Enter Song ID `1` ("Bohemian Rhapsody"). Song is added successfully.
   - Enter Song ID `2` ("Billie Jean"). Song is added successfully.
   - Now, attempt to add Song ID `1` again.
   - Show the message:
     ```
     [WARNING] Song 'Bohemian Rhapsody' (ID: 1) is already in the playlist!
     ```
   - Explain to the professor:
     > *"Notice that the application immediately rejected the duplicate addition. Internally, the Playlist class filters incoming tracks through a `std::unordered_set<int>` in $O(1)$ time complexity before appending them to the underlying sequential `std::vector`."*

---

### Minute 2:30 – 3:45 | Core Feature 2: Dynamic Shuffling & Sorting
1. **Add More Songs**:
   - Add Song ID `3` ("Hotel California") and Song ID `4` ("Shape of You").
   - View Playlist (Option `3`). Note the exact order: IDs `1, 2, 3, 4`.
2. **Perform Dynamic Shuffling (Requirement #2)**:
   - In Playlist Management, select option `7` (Shuffle Playlist).
   - Display the playlist again (Option `3`).
   - The sequence has altered (e.g., `3, 1, 4, 2`).
   - Explain to the professor:
     > *"Here we fulfill the second requirement. Using modern C++'s `std::shuffle` coupled with the Mersenne Twister engine (`std::mt19937`), the underlying indexed vector is dynamically reconfigured in a uniform, non-deterministic random permutation."*
3. **Demonstrate Sorting**:
   - Select option `8` (Sort Playlist) -> Sort by Duration or Title.
   - Show that the vector is ordered using `std::sort` with custom lambda comparator.

---

### Minute 3:45 – 5:00 | Core Feature 3: Playback Simulation & Polymorphism
1. **Load Playlist into Player**:
   - Go to Main Menu -> Type `3` (Music Player).
   - Select option `1` (Load Active Playlist).
2. **Demonstrate Playback Controls**:
   - Select `2` (Play): Shows current song playing, progress bar, duration.
   - Select `3` (Pause): Player state transitions to `PAUSED`.
   - Select `4` (Resume): Resumes playback.
   - Select `5` (Next Track) & `6` (Previous Track): Navigates through songs.
3. **Highlight OOP Concepts**:
   > *"The playback system isolates data models from playback hardware simulation. `MusicPlayer` implements the pure abstract interface `Playable`. The Application class interacts with the player strictly via the `Playable` contract."*

---

### Minute 5:00 – 6:00 | File Persistence & CSV Storage
1. **Save Playlist**:
   - Go to Main Menu -> Type `5` (Save Playlist to File).
   - Enter filename: `study_playlist.csv`.
   - Notification shows the playlist has been serialized to CSV.
2. **Reload Playlist**:
   - Main Menu -> Type `6` (Load Playlist from File).
   - Specify `study_playlist.csv`.
   - The playlist and its exact tracks are restored into memory.

---

### Minute 6:00 – 7:00 | Unit Tests & Concluding Remarks
1. **Exit the Application**:
   - Enter `0` to cleanly exit.
2. **Run Automated Test Suites**:
   ```powershell
   .\playlist_tests.exe
   .\storage_tests.exe
   ```
3. **Show Test Results**:
   > *"We have implemented 138 automated unit tests covering all edge cases: duplicate rejections, empty playlist guards, boundary navigation, file parsing errors, and shuffle entropy. 100% of tests pass."*
4. **Conclude**:
   > *"Thank you professors. We are now happy to answer any questions or show code implementations."*
