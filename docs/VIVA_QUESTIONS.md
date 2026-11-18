# Viva Questions & Answers — Smart Playlist Synthesizer

This guide contains **50+ expected viva and oral examination questions** covering Object-Oriented Programming theory, C++ implementation details, Standard Template Library (STL) components, and project-specific architecture.

---

## Category 1: Object-Oriented Programming (OOP) Fundamentals

### Q1. What are the four core pillars of OOP, and how are they used in this project?
- **Encapsulation**: Private attributes with public getters/setters in `Song`, `Playlist`, `MusicPlayer`, and `MusicLibrary`.
- **Abstraction**: `Playable` defines an abstract interface hiding playback implementation details from callers.
- **Inheritance**: `MusicPlayer` inherits from the abstract base class `Playable`.
- **Polymorphism**: Invoking `play()`, `pause()`, etc., via a `Playable*` pointer executes `MusicPlayer`'s overridden methods at runtime.

### Q2. What is Encapsulation, and why didn't you make `Song` attributes public?
Encapsulation binds data and the methods that manipulate that data together into a single unit (class), restricting direct unauthorized access. If `Song` attributes were public, any component could corrupt data (e.g., set duration to `-500` or ID to `0`), breaking invariants. Encapsulation enforces data integrity.

### Q3. What is an Abstract Class in C++?
An abstract class is a class that contains at least one **pure virtual function** (`virtual void func() = 0;`). It cannot be directly instantiated and acts as a contract for derived classes. In our project, `Playable` is an abstract class.

### Q4. What is Pure Virtual Function and how does it differ from a standard virtual function?
A standard virtual function provides a default implementation that derived classes can choose to override. A pure virtual function has no implementation in the base class (`= 0`) and **must** be overridden by concrete derived classes.

### Q5. Why is a virtual destructor necessary in a base class?
When an object of a derived class is deleted via a pointer to its base class (`Playable* p = new MusicPlayer(); delete p;`), a non-virtual destructor causes **undefined behavior**—only the base destructor executes, leading to resource leaks. A virtual destructor ensures the derived class destructor executes first, followed by the base class destructor.

### Q6. What is Runtime Polymorphism vs Compile-Time Polymorphism?
- **Compile-Time Polymorphism (Static Binding)**: Resolved at compile time via function overloading and operator overloading (e.g., `operator<<`, `operator==` in `Song`).
- **Runtime Polymorphism (Dynamic Binding)**: Resolved at runtime via virtual functions and vtables when invoking methods through base class pointers or references (`Playable*`).

### Q7. What is a Virtual Table (vtable) and Virtual Pointer (vptr)?
In C++, every class with virtual functions gets a compiler-generated **vtable** (an array of function pointers pointing to the virtual methods). Each instance of such a class contains a hidden pointer called **vptr** pointing to this table. When a virtual function is called through a pointer/reference, C++ dereferences the vptr to look up the correct address in the vtable.

### Q8. What is the difference between Composition and Inheritance?
- **Inheritance ("IS-A")**: `MusicPlayer` IS-A `Playable`. Used to model hierarchies and polymorphic contracts.
- **Composition ("HAS-A")**: `Playlist` HAS-A `std::vector<Song>`, and `Application` HAS-A `MusicLibrary`. Composition bundles independent objects as parts of a whole, offering lower coupling.

### Q9. Why prefer Composition over Inheritance ("Composition over Inheritance")?
Composition provides looser coupling, higher flexibility, and avoids deep, rigid inheritance hierarchies. In our system, `Playlist` does not inherit from `std::vector` (which also lacks a virtual destructor); instead, it contains a `std::vector` and a `std::unordered_set`.

---

## Category 2: C++ Language & Best Practices

### Q10. What is `const` correctness?
`const` correctness means marking member functions, parameters, and variables as `const` whenever they do not modify the object state (e.g., `getId() const`). This prevents accidental state mutation and allows functions to accept `const` references.

### Q11. Why pass parameters by `const` reference (`const std::string&`) instead of by value?
Passing by value makes a full copy of the string object and its underlying heap allocation. Passing by `const std::string&` passes only an 8-byte reference/pointer without copying, while `const` ensures the original cannot be modified.

### Q12. What is an `enum class` and why is it preferred over a plain `enum`?
A scoped enumeration (`enum class PlayState { STOPPED, PLAYING, PAUSED }`) avoids namespace pollution because its enumerators are scoped within the enum name (`PlayState::PLAYING`). It also prevents implicit conversion to integers.

### Q13. How does exception handling work in this project?
Constructors and validation logic throw standard exceptions like `std::invalid_argument` or `std::runtime_error`. The caller captures them using `try { ... } catch (const std::exception& e) { ... }`, preventing application crashes on invalid input.

### Q14. What are the Rule of Zero, Rule of Three, and Rule of Five in modern C++?
- **Rule of Three**: If a class manages raw resources and defines a destructor, copy constructor, or copy assignment operator, it likely needs all three.
- **Rule of Five**: In C++11, move constructor and move assignment operator were added.
- **Rule of Zero**: Prefer classes that manage no raw resources directly; use standard RAII wrappers (`std::vector`, `std::string`, `std::unique_ptr`), eliminating the need to write custom destructors or copy/move operations. Our project adheres to the Rule of Zero.

---

## Category 3: STL Containers & Data Structures

### Q15. Why does `Playlist` use both `std::vector` and `std::unordered_set`?
To solve Problem Statement 28:
- `std::vector<Song>` maintains sequential order, allows indexing (`[i]`), enables track sorting, and supports randomized shuffling.
- `std::unordered_set<int>` stores song IDs in a hash table to check if a track is already present in $O(1)$ average time before adding it.

### Q16. Why not use `std::vector` alone for duplicate checks?
Checking duplicates in a vector requires a linear scan ($O(N)$). For large playlists or repeated imports, checking duplicates in a vector takes $O(N^2)$ overall time. `std::unordered_set` reduces each check to $O(1)$ average time, resulting in $O(N)$ total time.

### Q17. What is the difference between `std::set` and `std::unordered_set`?
- `std::set`: Implemented as a Self-Balancing Red-Black Tree. Elements are kept sorted. Lookup, insertion, and deletion take $O(\log N)$.
- `std::unordered_set`: Implemented as a Hash Table. Elements are not sorted. Lookup, insertion, and deletion take $O(1)$ average time ($O(N)$ worst case if all hash keys collide).

### Q18. How does `std::shuffle` work in your project?
`std::shuffle` implements the Fisher-Yates shuffle algorithm on random-access iterators (`songs.begin()`, `songs.end()`). It swaps each element with an element chosen uniformly at random using the Mersenne Twister engine (`std::mt19937`).

### Q19. Why use `std::mt19937` instead of `rand()`?
`rand()` has a small period ($2^{31}-1$ or often only $32767$ in older implementations) and exhibits poor statistical distribution. `std::mt19937` (Mersenne Twister) has an enormous period of $2^{19937}-1$, providing uniform, high-quality pseudo-random numbers.

### Q20. What is the difference between `std::unordered_map` and `std::unordered_set`?
- `std::unordered_set<T>` stores unique keys only.
- `std::unordered_map<Key, Value>` stores unique key-value pairs. In `MusicLibrary`, we use `std::unordered_map<int, Song>` to map `songId -> Song`.

---

## Category 4: Project Features & Edge Cases

### Q21. How do you prevent duplicate songs when loading a playlist from a CSV file?
`PlaylistStorage::loadFromFile` attempts to insert each parsed song into the `Playlist` using `playlist.addSong(song)`. Since `addSong` internally checks `trackIds.find(id)`, duplicate records in the CSV file are rejected automatically.

### Q22. What happens if the user enters non-numeric input when a number is expected in the menu?
We implemented a robust input sanitation function:
```cpp
if (std::cin.fail()) {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    // Print invalid input message
}
```
This clears the stream error flags and discards invalid characters, preventing infinite loops.

### Q23. How does playback state work when an empty playlist is loaded?
`MusicPlayer::play()` checks `if (!currentPlaylist || currentPlaylist->isEmpty())`. If empty, it prints a message and transitions the player state to `PlayState::STOPPED` without crashing.

### Q24. How does `MusicPlayer` handle next track when the last track is playing?
The player checks whether `currentIndex + 1 >= songs.size()`. If wrap-around is enabled, it cycles back to index 0; otherwise, it stops playback and notifies the user that the playlist has ended.

---

## Category 5: Quick-Fire Technical Questions

| Question | Short Answer |
|----------|--------------|
| What is the size of an empty class in C++? | 1 byte (to ensure unique object addresses). |
| Can a constructor be virtual? | No, because an object's vtable is not initialized until the constructor finishes. |
| Can a destructor be pure virtual? | Yes, but it must still have a function body definition outside the class. |
| What does `override` specifier do? | Instructs compiler to verify that a method signature matches a base virtual function. |
| What is an iterator? | An object that points to an element inside an STL container, acting as a generalized pointer. |
| What is `std::move`? | Casts an lvalue to an rvalue reference, enabling resource transfer without copying. |
| Is `std::vector` thread-safe? | No, concurrent reads and writes require external synchronization. |
