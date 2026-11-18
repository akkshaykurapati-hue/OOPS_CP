# Smart Playlist Synthesizer — Full C++17 Web Application

> **OOP Course Project — SY B.Tech Engineering**
>
> A high-performance C++17 music library and playlist synthesizer with Object-Oriented architecture, transformed into a modern full-stack web application featuring an asynchronous REST API and a dark-themed music streaming dashboard.

---

## Team Members

| Name | Role |
|------|------|
| Akshay Kurapati | Team Member |
| Atharv Mane | Team Member |
| Manas Chaudhari | Team Member |
| Piyush Lasurkar | Team Member |
| Naman Kumar Rai | Team Member |
| RupeshKumar Nirankari | Team Member |

---

## Official Project Requirements

**Project No. 28 — Smart Playlist Synthesizer**

> *"Code a media tracking application preventing duplicate entries using specialized Sets, allowing dynamic shuffling mechanics by altering underlying indexed sequence configurations."*

### Achievable Requirements:
1. **HashSet Duplicate Prevention**: Filter raw track listings through a HashSet collection (`std::unordered_set<int>`) to enforce unique tracks within playlists and the library.
2. **ArrayList Bucket Shuffling**: Convert target list selections into sequential vector buckets (`std::vector<Song>`) to execute track shuffling with `std::shuffle` and `std::mt19937`.
3. **Architectural Isolation**: Enforce clean code boundaries by isolating data models (`Song`, `MusicLibrary`, `Playlist`, `PlaylistManager`) from simulated media playback (`Playable`, `MusicPlayer`) and persistence (`PlaylistStorage`).

---

## Architecture

The project now supports two standalone frontends powered by the same shared C++17 core OOP business logic:

1. **Original Console Application (`SmartPlaylistSynthesizer.exe`)**: Menu-driven terminal interface.
2. **Modern Web Application (`SmartPlaylistSynthesizerWeb.exe`)**: Full-stack application embedding a C++17 REST API server and serving a modern dark streaming dashboard.

```
                           ┌───────────────────────────┐
                           │      Browser Client       │
                           │  (HTML5 / CSS3 / Vanilla) │
                           └─────────────┬─────────────┘
                                         │ HTTP / JSON
                                         ▼
                           ┌───────────────────────────┐
                           │   WebServer (C++17 API)   │
                           │   (src/WebServer.cpp)     │
                           └─────────────┬─────────────┘
                                         │
 ┌───────────────────────┐               │
 │ Console Application   │               │
 │ (src/Application.cpp) │               │
 └───────────┬───────────┘               │
             │                           │
             └─────────────┬─────────────┘
                           ▼
          ┌──────────────────────────────────┐
          │     Shared Core Domain (OOP)     │
          ├──────────────────────────────────┤
          │ • Song                           │
          │ • MusicLibrary                   │
          │ • Playlist                       │
          │ • PlaylistManager                │
          │ • Playable (abstract)            │
          │ • MusicPlayer (simulated)        │
          │ • PlaylistStorage (CSV)          │
          └──────────────────────────────────┘
```

---

## Folder Structure

```
SmartPlaylistSynthesizer/
├── include/
│   ├── Application.h         # Console application controller
│   ├── MusicLibrary.h        # Master song catalog
│   ├── MusicPlayer.h         # Simulated audio playback
│   ├── Playable.h            # Abstract playback interface
│   ├── Playlist.h            # Vector + HashSet playlist
│   ├── PlaylistManager.h     # Multi-playlist management
│   ├── PlaylistStorage.h     # CSV file persistence
│   ├── Song.h                # Song track model
│   ├── WebServer.h           # C++17 HTTP REST API server
│   ├── httplib.h             # Lightweight HTTP server framework
│   └── nlohmann/
│       └── json.hpp          # JSON for Modern C++
├── src/
│   ├── Application.cpp       # Console application logic
│   ├── MusicLibrary.cpp      # MusicLibrary implementation
│   ├── MusicPlayer.cpp       # MusicPlayer implementation
│   ├── Playlist.cpp          # Playlist implementation
│   ├── PlaylistManager.cpp   # PlaylistManager implementation
│   ├── PlaylistStorage.cpp   # PlaylistStorage implementation
│   ├── Song.cpp              # Song implementation
│   ├── WebServer.cpp         # REST API route handlers & JSON serializers
│   ├── main.cpp              # Console entry point
│   └── web_main.cpp          # Web application server entry point
├── web/                      # Modern Dark Web Dashboard
│   ├── index.html            # Semantic HTML5 dashboard
│   ├── styles.css            # Dark theme CSS with glassmorphism & animations
│   └── app.js                # Vanilla JavaScript client (zero dependencies)
├── tests/
│   ├── playlist_tests.cpp    # 108 unit tests for core domain classes
│   ├── storage_tests.cpp     # 30 tests for CSV persistence & validation
│   └── api_tests.cpp         # Automated C++ HTTP REST API test suite
├── data/
│   └── sample_tracks.csv     # Sample track data
├── CMakeLists.txt            # Modern CMake build configuration
├── run_full_system_test.py   # 36 end-to-end integration tests over HTTP
├── IMPLEMENTATION_REPORT.md  # Detailed implementation report
└── README.md
```

---

## Prerequisites & Environment Setup

- **Operating System**: Windows 10/11
- **Toolchain**: MSYS2 UCRT64 GCC (C++17)
- **Windows Libraries**: `ws2_32` (Winsock), `crypt32` (both standard Windows libraries)

### MSYS2 UCRT64 Setup

To ensure you have the required GCC compiler tools installed in MSYS2 UCRT64:

```powershell
# Open PowerShell and verify g++
g++ --version
```

If not present, install via MSYS2:
```bash
pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-cmake
```

---

## Build & Execution Instructions

### 1. Build and Run the Web Application (Recommended)

#### Compile Web Application Executable:
```powershell
g++ -std=c++17 -Wall -Wextra -static -Iinclude src/web_main.cpp src/WebServer.cpp src/Song.cpp src/MusicLibrary.cpp src/Playlist.cpp src/PlaylistManager.cpp src/MusicPlayer.cpp src/PlaylistStorage.cpp -lws2_32 -lcrypt32 -o SmartPlaylistSynthesizerWeb.exe
```

#### Launch the Web Server:
```powershell
.\SmartPlaylistSynthesizerWeb.exe
```

#### Open the Dashboard in Your Browser:
Visit **[http://127.0.0.1:8080](http://127.0.0.1:8080)** (or `http://localhost:8080`).

---

### 2. Build and Run the Console Application

#### Compile Console Application:
```powershell
g++ -std=c++17 -Wall -Wextra -static -Iinclude src/main.cpp src/Application.cpp src/Song.cpp src/MusicLibrary.cpp src/Playlist.cpp src/PlaylistManager.cpp src/MusicPlayer.cpp src/PlaylistStorage.cpp -o SmartPlaylistSynthesizer.exe
```

#### Run Console Application:
```powershell
.\SmartPlaylistSynthesizer.exe
```

---

### 3. Build via CMake (Optional)

When CMake is configured in your environment:
```powershell
mkdir build
cd build
cmake .. -G "MinGW Makefiles"
cmake --build .
```

---

## Running Automated Tests

The project includes 3 layers of automated testing:

### 1. Playlist & Core Domain Unit Tests (108 Tests)
```powershell
g++ -std=c++17 -Wall -Wextra -static -Iinclude tests/playlist_tests.cpp src/Song.cpp src/MusicLibrary.cpp src/Playlist.cpp src/PlaylistManager.cpp src/MusicPlayer.cpp -o playlist_tests.exe
.\playlist_tests.exe
```
*Result: 108 passed, 0 failed.*

### 2. File Storage & CSV Persistence Tests (30 Tests)
```powershell
g++ -std=c++17 -Wall -Wextra -static -Iinclude tests/storage_tests.cpp src/Song.cpp src/Playlist.cpp src/PlaylistStorage.cpp -o storage_tests.exe
.\storage_tests.exe
```
*Result: 30 passed, 0 failed.*

### 3. C++ HTTP REST API Unit & Integration Tests (72 Tests)
Validates all endpoints including byte-range streaming (`206 Partial Content`), metadata updates, multipart audio upload, and duplicate rejection:
```powershell
cmake --build build --target api_tests
.\build\api_tests.exe
```
*Result: 72 passed, 0 failed.*

### 4. Full End-to-End System Integration Tests (58 Tests)
Validates all REST API endpoints, real audio streaming, byte-range requests, multipart uploads, duplicate rejections, simulated controls, and persistence across server restart:
```powershell
python run_full_system_test.py
```
*Result: 58 passed, 0 failed.*

---

## REST API Documentation

Base URL: `http://127.0.0.1:8080`

### System & Health
| Method | Endpoint | Description | Sample Response |
|--------|----------|-------------|-----------------|
| `GET` | `/api/health` | Service health, metrics, and audio engine status | `{"status":"ok","audioEngine":"HTML5 Real Audio Streaming","songsCount":10,"playlistsCount":2}` |

### Songs & Master Library
| Method | Endpoint | Description | Status Codes |
|--------|----------|-------------|--------------|
| `GET` | `/api/songs` | Retrieve master song catalog. Supports `?q=<term>` and `?sortBy=<title\|artist\|duration>` | `200 OK` |
| `GET` | `/api/songs/{id}` | Get detailed song metadata including `artworkUrl` and `hasAudio` | `200 OK`, `404 Not Found` |
| `POST` | `/api/songs` | Add new song to master library: `{"id":111,"title":"...","artist":"...","genre":"...","duration":210}` | `201 Created`, `400 Bad Request`, `409 Conflict` (duplicate ID) |
| `PUT` | `/api/songs/{id}` | Update song metadata: `{"title":"...","artist":"...","album":"...","genre":"...","duration":220}` | `200 OK`, `404 Not Found` |
| `DELETE` | `/api/songs/{id}` | Remove track from master library, all playlists, and disk | `200 OK`, `404 Not Found` |

### Audio Streaming & Upload
| Method | Endpoint | Description | Status Codes |
|--------|----------|-------------|--------------|
| `GET` | `/api/songs/{id}/audio` | Stream real audio file with byte-range support (`Range: bytes=start-end`) | `200 OK`, `206 Partial Content`, `404 Not Found` |
| `POST` | `/api/songs/upload` | Upload audio file (`multipart/form-data`) with magic byte validation (MP3, WAV, OGG, FLAC) | `201 Created`, `400 Bad Request`, `413 File Too Large` |

### Playlists
| Method | Endpoint | Description | Status Codes |
|--------|----------|-------------|--------------|
| `GET` | `/api/playlists` | List all playlist summaries | `200 OK` |
| `POST` | `/api/playlists` | Create new playlist: `{"name":"Late Night Mix"}` | `201 Created`, `409 Conflict` (duplicate name) |
| `GET` | `/api/playlists/{id}` | Get detailed playlist with track list | `200 OK`, `404 Not Found` |
| `PUT` | `/api/playlists/{id}` | Rename playlist: `{"name":"New Name"}` | `200 OK`, `409 Conflict` |
| `DELETE` | `/api/playlists/{id}` | Delete playlist | `200 OK`, `404 Not Found` |
| `POST` | `/api/playlists/{id}/songs` | Add song to playlist: `{"songId":101}`. **Enforces duplicate prevention via `std::unordered_set`** | `200 OK`, `409 Conflict` (duplicate detected) |
| `DELETE` | `/api/playlists/{id}/songs/{songId}` | Remove song from playlist | `200 OK`, `404 Not Found` |
| `POST` | `/api/playlists/{id}/shuffle` | **Shuffles playlist tracks using `std::shuffle` and `std::mt19937`** | `200 OK` |
| `POST` | `/api/playlists/{id}/sort` | Sort playlist tracks: `{"sortBy":"title\|artist\|duration"}` | `200 OK` |
| `POST` | `/api/playlists/{id}/save` | Persist playlist to CSV: `{"filename":"mix.csv"}` | `200 OK`, `500 Error` |
| `POST` | `/api/playlists/load` | Load playlist from CSV: `{"filename":"mix.csv"}` | `200 OK`, `404 Not Found` |

### Simulated Playback (Console & Backend Mirror)
| Method | Endpoint | Description | Status Codes |
|--------|----------|-------------|--------------|
| `GET` | `/api/player/status` | Current playback state, elapsed time, duration, and track info | `200 OK` |
| `POST` | `/api/player/play` | Start/resume playback: optional `{"playlistId":0,"trackIndex":0}` | `200 OK` |
| `POST` | `/api/player/pause` | Pause simulated playback | `200 OK` |
| `POST` | `/api/player/resume` | Resume simulated playback | `200 OK` |
| `POST` | `/api/player/next` | Advance to next track in active playlist | `200 OK` |
| `POST` | `/api/player/previous` | Return to previous track in active playlist | `200 OK` |
| `POST` | `/api/player/seek` | Seek simulated elapsed seconds: `{"seconds":45}` | `200 OK` |

---

## Troubleshooting

1. **Port already in use (`8080`)**:
   Run the web server on a different port using `--port`:
   ```powershell
   .\SmartPlaylistSynthesizerWeb.exe --port 8090
   ```
   Then visit `http://127.0.0.1:8090`.

2. **Windows Application Control or Antivirus warnings on newly compiled `.exe`**:
   Ensure binaries are compiled with `-static` flag as shown in the build commands above so no external runtime DLLs are missing.

3. **CSV files directory**:
   Saved and loaded playlists reside in `data/` by default. Ensure the directory exists or let the application create it automatically.
