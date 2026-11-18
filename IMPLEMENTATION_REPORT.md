# Implementation Report: Smart Playlist Synthesizer Web Application (Real Audio Streaming Upgrade)

**Date:** October 13, 2026  
**Project:** Smart Playlist Synthesizer (Course Project No. 28)  
**Authors:** Akshay Kurapati, Atharv Mane, Manas Chaudhari, Piyush Lasurkar, Naman Kumar Rai, RupeshKumar Nirankari  
**Target Environment:** Windows 11, MSYS2 UCRT64 GCC 16.2.0, C++17

---

## 1. Executive Summary

The **Smart Playlist Synthesizer** has been upgraded to support **native browser audio playback and user music file uploads**, operating as a complete, modern music-streaming application.

All existing C++ Object-Oriented core classes (`Song`, `MusicLibrary`, `Playlist`, `PlaylistManager`, `MusicPlayer`, `PlaylistStorage`) were preserved intact with backwards-compatible extensions. The original console executable (`SmartPlaylistSynthesizer.exe`) continues to compile and run independently, while the web executable (`SmartPlaylistSynthesizerWeb.exe`) serves real audio streaming, byte-range seek requests, multipart uploads, and a modern streaming dashboard.

---

## 2. Key Upgrades Implemented

### 1. Real Audio Streaming Engine
- **HTML5 Audio API**: Embedded `<audio id="realAudio">` synchronized with bottom player bar controls.
- **HTTP 206 Partial Content**: Implemented byte-range streaming via `Accept-Ranges: bytes` and `httplib::detail::mmap` content providers. Allows instantaneous seeking and buffering in Chrome, Firefox, and Edge.
- **Stereo WAV Audio Synthesis**: Built-in royalty-free procedural audio synthesizer generates stereo WAV audio files for default sample tracks (IDs 101–110) on initial server launch.
- **Auto-Advance & Repeat Modes**: Automatically plays the next song in the active playlist when current track finishes, with `repeat-off`, `repeat-all`, and `repeat-one` modes.

### 2. Audio File Upload & Format Validation
- **Multipart Form Upload (`POST /api/songs/upload`)**: Allows users to upload their own music files (`.mp3`, `.wav`, `.ogg`, `.flac`) via a drag-and-drop modal dialog.
- **Magic Byte Validation**: Validates file integrity at the binary level:
  - WAV (`RIFF....WAVE`)
  - MP3 (`ID3` header or sync frame `0xFF 0xE0`)
  - OGG (`OggS`)
  - FLAC (`fLaC`)
- **Safe Server Storage**: Uploaded files are assigned unique, collision-proof filenames in `data/audio/` with strict path traversal prevention (`..`, slashes rejected).
- **Duration Detection**: Automatically probes audio length from headers or estimated bitrates.

### 3. Persistent Library & Playlists Catalog
- **Restart Persistence**: Master song catalog and playlists manifest are persisted to `data/library_catalog.json` and `data/playlists_manifest.json`.
- **Survives Server Restarts**: Uploaded tracks, metadata edits, and custom playlists remain available across application restarts without requiring manual CSV saves.

### 4. Rich Modern Frontend Enhancements
- **Audio Scrubber**: Seekable progress bar showing elapsed time, remaining time, and buffered audio range.
- **Volume & Mute**: Smooth volume slider with mute toggle and volume memory.
- **Format Badges**: Format pills (e.g., `WAV • 44.1kHz`, `MP3 • 320kbps`, `FLAC • Lossless`) indicate audio quality.
- **Keyboard Shortcuts**: Spacebar (Play/Pause), Left/Right Arrows (Seek 5s), Up/Down Arrows (Volume), N (Next), P (Previous), S (Shuffle), R (Repeat).

---

## 3. Architecture & File Summary

```
SmartPlaylistSynthesizer/
├── include/
│   ├── Song.h                # Extended with album, artworkUrl, hasAudio
│   ├── MusicLibrary.h        # Extended with updateSong and clear
│   ├── Playlist.h            # Extended with updateSong
│   ├── PlaylistManager.h     # Extended with cascade update/remove and clear
│   ├── WebServer.h           # Extended with audio streaming, upload, persistence
│   ├── MusicPlayer.h         # Simulated audio playback (preserved)
│   ├── Playable.h            # Abstract playback interface (preserved)
│   ├── PlaylistStorage.h     # CSV file persistence (preserved)
│   ├── httplib.h             # C++17 HTTP server framework
│   └── nlohmann/json.hpp     # Modern JSON parser
├── src/
│   ├── Song.cpp              # Updated constructor and getters/setters
│   ├── MusicLibrary.cpp      # updateSong and clear implementation
│   ├── Playlist.cpp          # updateSong implementation
│   ├── PlaylistManager.cpp   # Cascade update and remove implementation
│   ├── WebServer.cpp         # Real audio synthesis, streaming, multipart upload
│   ├── web_main.cpp          # Web application server entry point
│   ├── Application.cpp       # Console application logic
│   └── main.cpp              # Console entry point
├── web/
│   ├── index.html            # Native <audio>, upload modal, edit modal, shortcuts modal
│   ├── styles.css            # Dark theme, dropzone styles, progress bar, equalizer
│   └── app.js                # HTML5 Audio event pipeline, drag-and-drop XHR upload
├── tests/
│   ├── playlist_tests.cpp    # 108 core unit tests
│   ├── storage_tests.cpp     # 30 storage persistence tests
│   └── api_tests.cpp         # 72 REST API and audio streaming tests
├── data/
│   ├── audio/                # Physical audio file storage (.wav, .mp3, .ogg, .flac)
│   ├── library_catalog.json  # Persistent master song registry
│   └── playlists_manifest.json # Persistent playlists manifest
├── run_full_system_test.py   # 58 end-to-end integration tests over HTTP
└── CMakeLists.txt            # CMake build configuration
```

---

## 4. Test Verification Results

All automated tests across all 4 suites were executed and passed with a 100% success rate:

| Test Suite | Execution Command | Tests Run | Passed | Failed | Result |
|------------|-------------------|-----------|--------|--------|--------|
| Playlist & Core OOP Domain | `.\build\playlist_tests.exe` | 108 | 108 | 0 | **100% PASS** |
| CSV Storage & Persistence | `.\build\storage_tests.exe` | 30 | 30 | 0 | **100% PASS** |
| C++ REST API & Audio Tests | `.\build\api_tests.exe` | 72 | 72 | 0 | **100% PASS** |
| Full E2E System Test Suite | `python run_full_system_test.py` | 58 | 58 | 0 | **100% PASS** |
| Browser Subagent E2E Test | Chrome Subagent Session | 8 user flows | 8 | 0 | **100% PASS** |
| **Total Automated Tests** | — | **268** | **268** | **0** | **100% PASS** |

### Verified Test Areas:
1. **HashSet Duplicate Prevention**: Verified duplicate track additions to playlists are strictly blocked with HTTP `409 Conflict`.
2. **Dynamic Vector Shuffling**: Verified playlist tracks are randomized via Mersenne Twister `std::mt19937` with 0 duplicate tracks and identical track count.
3. **Byte-Range Streaming**: Verified `GET /api/songs/:id/audio` with `Range: bytes=0-1023` responds with `206 Partial Content`, `Content-Range: bytes 0-1023/<total>`, and exact 1024-byte payload.
4. **Multipart Audio Upload**: Verified `POST /api/songs/upload` with valid WAV data creates track with `hasAudio: true`, assigns ID, streams audio, and cleans up on deletion.
5. **Format Validation**: Verified non-audio/corrupted files uploaded to the endpoint are rejected with `400 Bad Request`.
6. **Catalog Restart Persistence**: Verified that restarting `SmartPlaylistSynthesizerWeb.exe` preserves library tracks and playlists.
