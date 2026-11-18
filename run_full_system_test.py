import subprocess
import time
import urllib.request
import json
import sys
import os
import struct

TEST_PORT = 8088
BASE_URL = f"http://127.0.0.1:{TEST_PORT}"

total_tests = 0
passed_tests = 0
failed_tests = 0

def test(name, condition, details=""):
    global total_tests, passed_tests, failed_tests
    total_tests += 1
    if condition:
        passed_tests += 1
        print(f"  [PASS] {name}")
    else:
        failed_tests += 1
        print(f"  [FAIL] {name} - {details}")

def http_get(path):
    req = urllib.request.Request(f"{BASE_URL}{path}")
    try:
        with urllib.request.urlopen(req, timeout=5) as resp:
            data = resp.read()
            return resp.status, data
    except urllib.error.HTTPError as e:
        return e.code, e.read()

def http_get_with_headers(path, headers=None):
    req = urllib.request.Request(f"{BASE_URL}{path}", headers=headers or {})
    try:
        with urllib.request.urlopen(req, timeout=5) as resp:
            data = resp.read()
            return resp.status, data, dict(resp.headers)
    except urllib.error.HTTPError as e:
        return e.code, e.read(), dict(e.headers)

def http_post(path, payload=None):
    data_bytes = json.dumps(payload).encode('utf-8') if payload is not None else b"{}"
    req = urllib.request.Request(f"{BASE_URL}{path}", data=data_bytes, headers={'Content-Type': 'application/json'}, method='POST')
    try:
        with urllib.request.urlopen(req, timeout=5) as resp:
            data = resp.read()
            return resp.status, data
    except urllib.error.HTTPError as e:
        return e.code, e.read()

def http_post_raw(path, body, content_type):
    req = urllib.request.Request(f"{BASE_URL}{path}", data=body, headers={'Content-Type': content_type}, method='POST')
    try:
        with urllib.request.urlopen(req, timeout=5) as resp:
            data = resp.read()
            return resp.status, data, dict(resp.headers)
    except urllib.error.HTTPError as e:
        return e.code, e.read(), dict(e.headers)

def http_delete(path):
    req = urllib.request.Request(f"{BASE_URL}{path}", method='DELETE')
    try:
        with urllib.request.urlopen(req, timeout=5) as resp:
            data = resp.read()
            return resp.status, data
    except urllib.error.HTTPError as e:
        return e.code, e.read()

def http_put(path, payload=None):
    data_bytes = json.dumps(payload).encode('utf-8') if payload is not None else b"{}"
    req = urllib.request.Request(f"{BASE_URL}{path}", data=data_bytes, headers={'Content-Type': 'application/json'}, method='PUT')
    try:
        with urllib.request.urlopen(req, timeout=5) as resp:
            data = resp.read()
            return resp.status, data
    except urllib.error.HTTPError as e:
        return e.code, e.read()

def create_dummy_wav(duration_sec=1, sample_rate=8000):
    num_samples = int(duration_sec * sample_rate)
    num_channels = 1
    bits_per_sample = 16
    byte_rate = sample_rate * num_channels * (bits_per_sample // 8)
    block_align = num_channels * (bits_per_sample // 8)
    data_size = num_samples * block_align
    chunk_size = 36 + data_size

    header = struct.pack(
        '<4sI4s4sIHHIIHH4sI',
        b'RIFF',
        chunk_size,
        b'WAVE',
        b'fmt ',
        16,
        1, # PCM
        num_channels,
        sample_rate,
        byte_rate,
        block_align,
        bits_per_sample,
        b'data',
        data_size
    )
    samples = bytearray(data_size)
    return header + bytes(samples)

def main():
    print("\n========================================================")
    print("  Smart Playlist Synthesizer — Full System Test Suite")
    print("========================================================\n")

    # Start the C++ Web Application Server in background
    print("Starting SmartPlaylistSynthesizerWeb.exe on port", TEST_PORT, "...")
    server_proc = subprocess.Popen(
        ["./SmartPlaylistSynthesizerWeb.exe", "--port", str(TEST_PORT), "--host", "127.0.0.1", "--web-root", "./web"],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )

    time.sleep(1.0) # Wait for server socket bind

    try:
        # ── 1. Static Asset Serving ────────────────────────────────────
        print("\n--- 1. Static File Serving ---")
        status, data = http_get("/")
        test("GET / (index.html)", status == 200 and b"Smart Playlist Synthesizer" in data)

        status, data = http_get("/styles.css")
        test("GET /styles.css", status == 200 and b"--bg-base" in data)

        status, data = http_get("/app.js")
        test("GET /app.js", status == 200 and b"Smart Playlist Synthesizer" in data)

        # ── 2. Health Check API ────────────────────────────────────────
        print("\n--- 2. Health Endpoint ---")
        status, data = http_get("/api/health")
        j = json.loads(data.decode('utf-8'))
        test("GET /api/health responds 200", status == 200)
        test("Health status is 'ok'", j.get("status") == "ok")
        test("Health reports songsCount >= 10", j.get("songsCount", 0) >= 10)
        test("Health reports playlistsCount >= 2", j.get("playlistsCount", 0) >= 2)
        test("Audio engine reports real HTML5 streaming", "HTML5 Real Audio Streaming" in j.get("audioEngine", ""))

        # ── 3. Songs & Search API ──────────────────────────────────────
        print("\n--- 3. Song Library API ---")
        status, data = http_get("/api/songs")
        songs = json.loads(data.decode('utf-8'))
        test("GET /api/songs returns array", status == 200 and isinstance(songs, list))
        test("Sample tracks loaded (>= 10)", len(songs) >= 10)

        # Search query
        status, data = http_get("/api/songs?q=Midnight")
        search_res = json.loads(data.decode('utf-8'))
        test("GET /api/songs?q=Midnight finds track", status == 200 and len(search_res) >= 1 and "Midnight" in search_res[0]["title"])

        # Add custom track
        status, data = http_post("/api/songs", {
            "id": 555,
            "title": "Integration Anthem",
            "artist": "Test Orchestra",
            "genre": "Classical Synth",
            "duration": 240
        })
        test("POST /api/songs creates track (201)", status == 201)

        # Prevent duplicate ID in master library
        status, data = http_post("/api/songs", {
            "id": 555,
            "title": "Duplicate Anthem",
            "artist": "Another Artist",
            "genre": "Rock",
            "duration": 180
        })
        test("POST /api/songs rejects duplicate ID (409)", status == 409)

        # ── 4. Playlist Lifecycle & Duplicate Prevention ───────────────
        print("\n--- 4. Playlist Management & Duplicate Prevention ---")
        status, data = http_post("/api/playlists", {"name": f"Test Integration Playlist {int(time.time())}"})
        new_pl = json.loads(data.decode('utf-8'))
        pl_id = new_pl.get("id")
        test("POST /api/playlists creates playlist (201)", status == 201 and pl_id is not None)

        # Reject duplicate playlist name
        status, data = http_post("/api/playlists", {"name": new_pl.get("name")})
        test("POST /api/playlists rejects duplicate name (409)", status == 409)

        # Add song 101 to playlist
        status, data = http_post(f"/api/playlists/{pl_id}/songs", {"songId": 101})
        test("POST /api/playlists/{id}/songs adds song 101", status == 200)

        # CRITICAL TEST: Attempt to add song 101 again (duplicate prevention!)
        status, data = http_post(f"/api/playlists/{pl_id}/songs", {"songId": 101})
        err_j = json.loads(data.decode('utf-8'))
        test("Duplicate song in playlist REJECTED (409 Conflict)", status == 409)
        test("Error message explains duplicate rejection", "error" in err_j and "Duplicate track" in err_j["error"])

        # Add song 102
        status, data = http_post(f"/api/playlists/{pl_id}/songs", {"songId": 102})
        test("POST /api/playlists/{id}/songs adds song 102", status == 200)

        # Inspect playlist details
        status, data = http_get(f"/api/playlists/{pl_id}")
        pl_details = json.loads(data.decode('utf-8'))
        test("GET /api/playlists/{id} has 2 tracks", status == 200 and pl_details.get("trackCount") == 2)

        # Test Shuffling (Mandatory requirement)
        status, data = http_post(f"/api/playlists/{pl_id}/shuffle")
        shuffled = json.loads(data.decode('utf-8'))
        test("POST /api/playlists/{id}/shuffle returns 200", status == 200 and shuffled.get("trackCount") == 2)

        # Test Sorting
        status, data = http_post(f"/api/playlists/{pl_id}/sort", {"sortBy": "title"})
        test("POST /api/playlists/{id}/sort returns 200", status == 200)

        # Rename playlist
        status, data = http_put(f"/api/playlists/{pl_id}", {"name": f"Renamed Playlist {int(time.time())}"})
        test("PUT /api/playlists/{id} renames playlist", status == 200)

        # ── 5. File Persistence (Save & Load) ──────────────────────────
        print("\n--- 5. Storage API (Save/Load) ---")
        status, data = http_post(f"/api/playlists/{pl_id}/save", {"filename": "full_test_playlist.csv"})
        save_res = json.loads(data.decode('utf-8'))
        test("POST /api/playlists/{id}/save saves to CSV", status == 200 and save_res.get("status") == "saved")

        status, data = http_post("/api/playlists/load", {"filename": "full_test_playlist.csv"})
        load_res = json.loads(data.decode('utf-8'))
        test("POST /api/playlists/load loads from CSV", status == 200 and load_res.get("trackCount") == 2)

        # ── 6. Simulated Playback Controls ─────────────────────────────
        print("\n--- 6. Simulated Player Navigation ---")
        # Play track
        status, data = http_post("/api/player/play", {"playlistId": pl_id, "trackIndex": 0})
        p_status = json.loads(data.decode('utf-8'))
        test("POST /api/player/play initiates playback", status == 200 and p_status.get("state") == "PLAYING")
        test("Player indicates simulated mode", p_status.get("isSimulated") == True)
        test("Active track metadata returned", p_status.get("track") is not None)

        # Pause
        status, data = http_post("/api/player/pause")
        p_status = json.loads(data.decode('utf-8'))
        test("POST /api/player/pause sets PAUSED state", status == 200 and p_status.get("state") == "PAUSED")

        # Resume
        status, data = http_post("/api/player/resume")
        p_status = json.loads(data.decode('utf-8'))
        test("POST /api/player/resume resumes PLAYING state", status == 200 and p_status.get("state") == "PLAYING")

        # Next track
        status, data = http_post("/api/player/next")
        p_status = json.loads(data.decode('utf-8'))
        test("POST /api/player/next advances track index", status == 200 and p_status.get("currentTrackIndex") == 1)

        # Previous track
        status, data = http_post("/api/player/previous")
        p_status = json.loads(data.decode('utf-8'))
        test("POST /api/player/previous returns to track 0", status == 200 and p_status.get("currentTrackIndex") == 0)

        # Seek
        status, data = http_post("/api/player/seek", {"seconds": 40})
        p_status = json.loads(data.decode('utf-8'))
        test("POST /api/player/seek updates elapsed time", status == 200 and p_status.get("elapsedSeconds", 0) >= 40)

        # Status query
        status, data = http_get("/api/player/status")
        p_status = json.loads(data.decode('utf-8'))
        test("GET /api/player/status returns accurate state", status == 200 and p_status.get("durationSeconds", 0) > 0)

        # ── 7. Real Audio Streaming & Byte-Range Requests ──────────────
        print("\n--- 7. Real Audio Streaming & Range Requests ---")
        status, data, headers = http_get_with_headers("/api/songs/101/audio")
        test("GET /api/songs/101/audio responds 200", status == 200)
        test("Content-Type is audio/wav", headers.get("Content-Type", headers.get("content-type")) == "audio/wav")
        test("Accept-Ranges: bytes supported", headers.get("Accept-Ranges", headers.get("accept-ranges")) == "bytes")
        test("Audio data payload length > 1000 bytes", len(data) > 1000)

        # Range request (HTTP 206 Partial Content)
        status, data, headers = http_get_with_headers("/api/songs/101/audio", {"Range": "bytes=0-1023"})
        test("Range GET responds 206 Partial Content", status == 206)
        test("Partial content length is exactly 1024 bytes", len(data) == 1024)
        cr_header = headers.get("Content-Range", headers.get("content-range", ""))
        test("Content-Range header starts with 'bytes 0-1023/'", cr_header.startswith("bytes 0-1023/"))

        # Non-existent audio returns 404
        status, data = http_get("/api/songs/99999/audio")
        test("Non-existent track audio responds 404", status == 404)

        # ── 8. Track Metadata Updates (PUT) ────────────────────────────
        print("\n--- 8. Track Metadata Update API ---")
        status, data = http_get("/api/songs/101")
        song_101 = json.loads(data.decode('utf-8'))
        test("GET /api/songs/101 responds 200", status == 200)
        test("Song 101 hasAudio is true", song_101.get("hasAudio") == True)
        test("Song 101 has valid audioUrl", song_101.get("audioUrl") == "/api/songs/101/audio")

        status, data = http_put("/api/songs/101", {
            "title": "Midnight Drive (2026 Remaster)",
            "artist": "Neon Runner",
            "album": "Synth Horizons",
            "genre": "Retrowave",
            "duration": 224,
            "artworkUrl": "https://images.unsplash.com/photo-1518709268805-4e9042af9f23?w=300"
        })
        updated_song = json.loads(data.decode('utf-8'))
        test("PUT /api/songs/101 updates metadata (200)", status == 200 and updated_song.get("album") == "Synth Horizons")

        # ── 9. Real Audio File Upload & Validation ─────────────────────
        print("\n--- 9. Audio Upload & Validation API ---")
        boundary = "----PythonTestBoundary777"
        wav_bytes = create_dummy_wav(duration_sec=2, sample_rate=8000)

        body_parts = [
            f"--{boundary}\r\nContent-Disposition: form-data; name=\"title\"\r\n\r\nPython Uploaded Track\r\n".encode('utf-8'),
            f"--{boundary}\r\nContent-Disposition: form-data; name=\"artist\"\r\n\r\nPython Tester\r\n".encode('utf-8'),
            f"--{boundary}\r\nContent-Disposition: form-data; name=\"genre\"\r\n\r\nAmbient Synth\r\n".encode('utf-8'),
            f"--{boundary}\r\nContent-Disposition: form-data; name=\"file\"; filename=\"synth_beat.wav\"\r\nContent-Type: audio/wav\r\n\r\n".encode('utf-8'),
            wav_bytes,
            f"\r\n--{boundary}--\r\n".encode('utf-8')
        ]
        multipart_body = b"".join(body_parts)

        status, data, _ = http_post_raw("/api/songs/upload", multipart_body, f"multipart/form-data; boundary={boundary}")
        upload_resp = json.loads(data.decode('utf-8')) if status == 201 else {}
        test("POST /api/songs/upload responds 201 Created", status == 201)
        up_id = upload_resp.get("id")
        test("Uploaded track assigned ID >= 200", up_id is not None and up_id >= 200)
        test("Uploaded track hasAudio is true", upload_resp.get("hasAudio") == True)

        # Stream newly uploaded file
        status, data, _ = http_get_with_headers(f"/api/songs/{up_id}/audio")
        test(f"Stream uploaded track /api/songs/{up_id}/audio responds 200", status == 200 and len(data) == len(wav_bytes))

        # Reject corrupted/non-audio upload
        bad_parts = [
            f"--{boundary}\r\nContent-Disposition: form-data; name=\"title\"\r\n\r\nCorrupted Track\r\n".encode('utf-8'),
            f"--{boundary}\r\nContent-Disposition: form-data; name=\"file\"; filename=\"bad.txt\"\r\nContent-Type: text/plain\r\n\r\nThis is plain text\r\n".encode('utf-8'),
            f"--{boundary}--\r\n".encode('utf-8')
        ]
        bad_status, bad_data, _ = http_post_raw("/api/songs/upload", b"".join(bad_parts), f"multipart/form-data; boundary={boundary}")
        test("Upload non-audio file rejected with 400", bad_status == 400)

        # Delete uploaded track
        status, _ = http_delete(f"/api/songs/{up_id}")
        test("DELETE uploaded song succeeds (200)", status == 200)
        status, _ = http_get(f"/api/songs/{up_id}/audio")
        test("Audio URL after deletion returns 404", status == 404)

        # ── 10. Cleanup & Teardown ─────────────────────────────────────
        print("\n--- 10. Cleanup & Teardown ---")
        status, data = http_delete(f"/api/playlists/{pl_id}/songs/101")
        test("DELETE /api/playlists/{id}/songs/101 removes track", status == 200)

        status, data = http_delete(f"/api/playlists/{pl_id}")
        test("DELETE /api/playlists/{id} deletes playlist", status == 200)

        status, data = http_delete("/api/songs/555")
        test("DELETE /api/songs/555 deletes track from library", status == 200)

    finally:
        print("\nStopping C++ Web Server process...")
        server_proc.terminate()
        try:
            server_proc.wait(timeout=3)
        except subprocess.TimeoutExpired:
            server_proc.kill()

    # ── 11. Persistence Across Server Restart ──────────────────────────
    print("\n--- 11. Server Persistence Check (Restart Test) ---")
    restart_proc = subprocess.Popen(
        ["./SmartPlaylistSynthesizerWeb.exe", "--port", str(TEST_PORT), "--host", "127.0.0.1", "--web-root", "./web"],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )
    time.sleep(1.0)
    try:
        status, data = http_get("/api/songs")
        songs_after_restart = json.loads(data.decode('utf-8'))
        test("Library persists across server restart (>= 10 songs)", status == 200 and len(songs_after_restart) >= 10)

        status, data = http_get("/api/playlists")
        pls_after_restart = json.loads(data.decode('utf-8'))
        test("Playlists persist across server restart (>= 2 playlists)", status == 200 and len(pls_after_restart) >= 2)
    finally:
        print("Stopping restarted C++ Web Server process...")
        restart_proc.terminate()
        try:
            restart_proc.wait(timeout=3)
        except subprocess.TimeoutExpired:
            restart_proc.kill()

    print("\n========================================================")
    print(f"  Results: {passed_tests} passed, {failed_tests} failed, {total_tests} total")
    print("========================================================\n")

    if failed_tests > 0:
        sys.exit(1)

if __name__ == "__main__":
    main()
