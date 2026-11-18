/**
 * @file api_tests.cpp
 * @brief Automated unit and integration tests for the WebServer REST API.
 *
 * Verifies all API endpoints:
 *   - GET /api/health
 *   - GET /api/songs & search query
 *   - POST /api/songs
 *   - DELETE /api/songs/{id}
 *   - GET /api/playlists & POST /api/playlists
 *   - GET /api/playlists/{id}
 *   - POST /api/playlists/{id}/songs (duplicate prevention)
 *   - DELETE /api/playlists/{id}/songs/{songId}
 *   - POST /api/playlists/{id}/shuffle
 *   - POST /api/playlists/{id}/sort
 *   - POST /api/playlists/{id}/save & POST /api/playlists/load
 *   - Simulated player: play, pause, resume, next, previous, seek, status
 */

#include "WebServer.h"
#include <iostream>
#include <cassert>
#include <thread>
#include <chrono>
#include "httplib.h"
#include "nlohmann/json.hpp"

static int totalTests  = 0;
static int passedTests = 0;
static int failedTests = 0;

#define TEST(name, expr)                                            \
    do {                                                            \
        ++totalTests;                                               \
        if (expr) {                                                 \
            ++passedTests;                                          \
            std::cout << "  [PASS] " << name << "\n";              \
        } else {                                                    \
            ++failedTests;                                          \
            std::cout << "  [FAIL] " << name                       \
                      << " (line " << __LINE__ << ")\n";            \
        }                                                           \
    } while (0)

int main() {
    std::cout << "\n";
    std::cout << "  ====================================================\n";
    std::cout << "  Smart Playlist Synthesizer — REST API Tests\n";
    std::cout << "  ====================================================\n\n";

    const int testPort = 8089;
    WebServer server(testPort, "127.0.0.1", "./web");
    server.init();

    std::thread serverThread([&server]() {
        server.start();
    });

    // Allow server to bind and start listening
    std::this_thread::sleep_for(std::chrono::milliseconds(400));

    httplib::Client cli("127.0.0.1", testPort);
    cli.set_connection_timeout(2, 0);
    cli.set_read_timeout(2, 0);

    // ── 1. Health Endpoint ────────────────────────────────────────────
    std::cout << "  --- Health Check Tests ---\n";
    {
        auto res = cli.Get("/api/health");
        TEST("Health check responds 200", res && res->status == 200);
        if (res && res->status == 200) {
            auto j = nlohmann::json::parse(res->body);
            TEST("Health status is 'ok'", j["status"] == "ok");
            TEST("Health app title present", j["app"] == "Smart Playlist Synthesizer");
            TEST("Health reports songs count >= 10", j["songsCount"].get<int>() >= 10);
            TEST("Health reports playlists count >= 2", j["playlistsCount"].get<int>() >= 2);
        }
    }

    // ── 2. Song Retrieval & Search ────────────────────────────────────
    std::cout << "\n  --- Song Library Tests ---\n";
    {
        auto res = cli.Get("/api/songs");
        TEST("Get all songs responds 200", res && res->status == 200);
        if (res && res->status == 200) {
            auto j = nlohmann::json::parse(res->body);
            TEST("Library returns array of songs", j.is_array());
            TEST("Library has at least 10 sample songs", j.size() >= 10);
        }

        // Test search query
        auto searchRes = cli.Get("/api/songs?q=midnight");
        TEST("Search query responds 200", searchRes && searchRes->status == 200);
        if (searchRes && searchRes->status == 200) {
            auto j = nlohmann::json::parse(searchRes->body);
            TEST("Search finds Midnight Drive", j.size() >= 1 && j[0]["title"].get<std::string>().find("Midnight") != std::string::npos);
        }

        // Add custom song
        nlohmann::json newSong = {
            {"id", 999},
            {"title", "API Test Track"},
            {"artist", "Unit Tester"},
            {"genre", "Electro"},
            {"duration", 180}
        };
        auto addRes = cli.Post("/api/songs", newSong.dump(), "application/json");
        TEST("Add new song responds 201 Created", addRes && addRes->status == 201);

        // Reject duplicate song ID
        auto dupRes = cli.Post("/api/songs", newSong.dump(), "application/json");
        TEST("Duplicate song ID rejected with 409 Conflict", dupRes && dupRes->status == 409);

        // Remove the test song
        auto delRes = cli.Delete("/api/songs/999");
        TEST("Delete song responds 200", delRes && delRes->status == 200);
    }

    // ── 3. Playlist Management & Duplicate Prevention ─────────────────
    std::cout << "\n  --- Playlist & Duplicate Prevention Tests ---\n";
    {
        // Get playlists
        auto plRes = cli.Get("/api/playlists");
        TEST("Get playlists responds 200", plRes && plRes->status == 200);
        int initialPlCount = 0;
        if (plRes && plRes->status == 200) {
            auto j = nlohmann::json::parse(plRes->body);
            initialPlCount = static_cast<int>(j.size());
            TEST("Initial playlists count >= 2", initialPlCount >= 2);
        }

        // Create new playlist with unique timestamped name
        std::string testPlName = "Test Mix " + std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
        nlohmann::json plPayload = {{"name", testPlName}};
        auto createRes = cli.Post("/api/playlists", plPayload.dump(), "application/json");
        TEST("Create playlist responds 201 Created", createRes && createRes->status == 201);
        int testPlId = -1;
        if (createRes && createRes->status == 201) {
            auto j = nlohmann::json::parse(createRes->body);
            testPlId = j["id"].get<int>();
            TEST("Created playlist has valid ID", testPlId >= 0);
            TEST("Created playlist name matches", j["name"] == testPlName);
        }

        // Reject duplicate playlist name
        auto dupPlRes = cli.Post("/api/playlists", plPayload.dump(), "application/json");
        TEST("Duplicate playlist name rejected with 409", dupPlRes && dupPlRes->status == 409);

        // Add Song 101 to new playlist
        nlohmann::json addTrackPayload = {{"songId", 101}};
        std::string addPath = "/api/playlists/" + std::to_string(testPlId) + "/songs";
        auto addSongRes = cli.Post(addPath.c_str(), addTrackPayload.dump(), "application/json");
        TEST("Add song 101 to playlist responds 200", addSongRes && addSongRes->status == 200);

        // CRITICAL REQUIREMENT: Attempt to add duplicate song 101 into same playlist!
        auto dupSongRes = cli.Post(addPath.c_str(), addTrackPayload.dump(), "application/json");
        TEST("Duplicate song in playlist REJECTED with 409 Conflict", dupSongRes && dupSongRes->status == 409);
        if (dupSongRes && dupSongRes->status == 409) {
            auto j = nlohmann::json::parse(dupSongRes->body);
            TEST("Duplicate error explanation returned", j.contains("error"));
        }

        // Add Song 102
        nlohmann::json addTrack2 = {{"songId", 102}};
        cli.Post(addPath.c_str(), addTrack2.dump(), "application/json");

        // Verify playlist details
        std::string detailPath = "/api/playlists/" + std::to_string(testPlId);
        auto detailRes = cli.Get(detailPath.c_str());
        TEST("Get playlist details responds 200", detailRes && detailRes->status == 200);
        if (detailRes && detailRes->status == 200) {
            auto j = nlohmann::json::parse(detailRes->body);
            TEST("Track count is 2", j["trackCount"].get<int>() == 2);
            TEST("Tracks array length is 2", j["tracks"].size() == 2);
        }

        // Test Shuffling (Official Requirement)
        std::string shufflePath = "/api/playlists/" + std::to_string(testPlId) + "/shuffle";
        auto shuffleRes = cli.Post(shufflePath.c_str(), "{}", "application/json");
        TEST("Shuffle playlist responds 200", shuffleRes && shuffleRes->status == 200);
        if (shuffleRes && shuffleRes->status == 200) {
            auto j = nlohmann::json::parse(shuffleRes->body);
            TEST("Track count preserved after shuffle", j["trackCount"].get<int>() == 2);
        }

        // Remove song 101
        std::string removePath = "/api/playlists/" + std::to_string(testPlId) + "/songs/101";
        auto removeRes = cli.Delete(removePath.c_str());
        TEST("Remove song 101 from playlist responds 200", removeRes && removeRes->status == 200);

        // Delete test playlist
        std::string delPlPath = "/api/playlists/" + std::to_string(testPlId);
        auto delPlRes = cli.Delete(delPlPath.c_str());
        TEST("Delete test playlist responds 200", delPlRes && delPlRes->status == 200);
    }

    // ── 4. Storage Save and Load ──────────────────────────────────────
    std::cout << "\n  --- Persistence (Save/Load) Tests ---\n";
    {
        std::string savePath = "/api/playlists/0/save";
        nlohmann::json savePayload = {{"filename", "test_api_saved.csv"}};
        auto saveRes = cli.Post(savePath.c_str(), savePayload.dump(), "application/json");
        TEST("Save playlist responds 200", saveRes && saveRes->status == 200);

        nlohmann::json loadPayload = {{"filename", "test_api_saved.csv"}};
        auto loadRes = cli.Post("/api/playlists/load", loadPayload.dump(), "application/json");
        TEST("Load playlist responds 200", loadRes && loadRes->status == 200);
        if (loadRes && loadRes->status == 200) {
            auto j = nlohmann::json::parse(loadRes->body);
            TEST("Loaded playlist has tracks", j["trackCount"].get<int>() > 0);
        }
    }

    // ── 5. Simulated Player Navigation ────────────────────────────────
    std::cout << "\n  --- Simulated Player Tests ---\n";
    {
        // Initial status
        auto statusRes = cli.Get("/api/player/status");
        TEST("Player status responds 200", statusRes && statusRes->status == 200);

        // Play active playlist
        nlohmann::json playPayload = {{"playlistId", 0}, {"trackIndex", 0}};
        auto playRes = cli.Post("/api/player/play", playPayload.dump(), "application/json");
        TEST("Play command responds 200", playRes && playRes->status == 200);
        if (playRes && playRes->status == 200) {
            auto j = nlohmann::json::parse(playRes->body);
            TEST("State is PLAYING", j["state"] == "PLAYING");
            TEST("Track details present", j.contains("track") && !j["track"].is_null());
            TEST("isSimulated flag is true", j["isSimulated"] == true);
        }

        // Pause
        auto pauseRes = cli.Post("/api/player/pause", "{}", "application/json");
        TEST("Pause command responds 200", pauseRes && pauseRes->status == 200);
        if (pauseRes && pauseRes->status == 200) {
            auto j = nlohmann::json::parse(pauseRes->body);
            TEST("State is PAUSED", j["state"] == "PAUSED");
        }

        // Resume
        auto resumeRes = cli.Post("/api/player/resume", "{}", "application/json");
        TEST("Resume command responds 200", resumeRes && resumeRes->status == 200);
        if (resumeRes && resumeRes->status == 200) {
            auto j = nlohmann::json::parse(resumeRes->body);
            TEST("State back to PLAYING", j["state"] == "PLAYING");
        }

        // Next track
        auto nextRes = cli.Post("/api/player/next", "{}", "application/json");
        TEST("Next track responds 200", nextRes && nextRes->status == 200);
        if (nextRes && nextRes->status == 200) {
            auto j = nlohmann::json::parse(nextRes->body);
            TEST("Current track index moved to 1", j["currentTrackIndex"].get<int>() == 1);
        }

        // Previous track
        auto prevRes = cli.Post("/api/player/previous", "{}", "application/json");
        TEST("Previous track responds 200", prevRes && prevRes->status == 200);
        if (prevRes && prevRes->status == 200) {
            auto j = nlohmann::json::parse(prevRes->body);
            TEST("Current track index back to 0", j["currentTrackIndex"].get<int>() == 0);
        }

        // Simulated seek
        nlohmann::json seekPayload = {{"seconds", 45}};
        auto seekRes = cli.Post("/api/player/seek", seekPayload.dump(), "application/json");
        TEST("Seek responds 200", seekRes && seekRes->status == 200);
        if (seekRes && seekRes->status == 200) {
            auto j = nlohmann::json::parse(seekRes->body);
            TEST("Elapsed seconds updated to >= 45", j["elapsedSeconds"].get<int>() >= 45);
        }
    }

    // ── 6. Audio Streaming & Byte-Range Requests ──────────────────────
    std::cout << "\n  --- Audio Streaming & Byte-Range Tests ---\n";
    {
        // 1. Full GET of sample track 101
        auto streamRes = cli.Get("/api/songs/101/audio");
        TEST("Stream audio responds 200", streamRes && streamRes->status == 200);
        if (streamRes && streamRes->status == 200) {
            TEST("Audio content type is audio/wav", streamRes->get_header_value("Content-Type") == "audio/wav");
            TEST("Accept-Ranges header present", streamRes->get_header_value("Accept-Ranges") == "bytes");
            TEST("Audio body contains data", !streamRes->body.empty());
        }

        // 2. Partial Content (HTTP 206) with Range header
        httplib::Headers rangeHeader = {{"Range", "bytes=0-511"}};
        auto rangeRes = cli.Get("/api/songs/101/audio", rangeHeader);
        TEST("Range request responds 206 Partial Content", rangeRes && rangeRes->status == 206);
        if (rangeRes && rangeRes->status == 206) {
            TEST("Partial content length is 512 bytes", rangeRes->body.size() == 512);
            std::string contentRange = rangeRes->get_header_value("Content-Range");
            TEST("Content-Range header starts with 'bytes 0-511/'", contentRange.rfind("bytes 0-511/", 0) == 0);
        }

        // 3. Audio for non-existent track returns 404
        auto missingRes = cli.Get("/api/songs/99999/audio");
        TEST("Missing audio returns 404", missingRes && missingRes->status == 404);
    }

    // ── 7. Song Metadata Get and Update (PUT) ─────────────────────────
    std::cout << "\n  --- Song Metadata Update Tests ---\n";
    {
        // Fetch song 101
        auto getRes = cli.Get("/api/songs/101");
        TEST("Get song 101 responds 200", getRes && getRes->status == 200);
        if (getRes && getRes->status == 200) {
            auto j = nlohmann::json::parse(getRes->body);
            TEST("Song 101 has audio flag true", j["hasAudio"] == true);
            TEST("Song 101 has valid audioUrl", j["audioUrl"] == "/api/songs/101/audio");
        }

        // Update song 101 metadata
        nlohmann::json updatePayload = {
            {"title", "Midnight Drive (2026 Remaster)"},
            {"artist", "Neon Dreamer"},
            {"album", "Synth Odyssey"},
            {"genre", "Retrowave"},
            {"duration", 225},
            {"artworkUrl", "https://example.com/art.jpg"}
        };
        auto putRes = cli.Put("/api/songs/101", updatePayload.dump(), "application/json");
        TEST("PUT /api/songs/101 responds 200", putRes && putRes->status == 200);
        if (putRes && putRes->status == 200) {
            auto j = nlohmann::json::parse(putRes->body);
            TEST("Updated title reflected", j["title"] == "Midnight Drive (2026 Remaster)");
            TEST("Updated album reflected", j["album"] == "Synth Odyssey");
        }

        // Verify with GET
        auto verifyRes = cli.Get("/api/songs/101");
        if (verifyRes && verifyRes->status == 200) {
            auto j = nlohmann::json::parse(verifyRes->body);
            TEST("GET /api/songs/101 returns updated title", j["title"] == "Midnight Drive (2026 Remaster)");
        }
    }

    // ── 8. Multipart Audio File Upload & Deletion ──────────────────────
    std::cout << "\n  --- Audio File Upload & Validation Tests ---\n";
    {
        // Synthesize minimal valid 44-byte WAV header + 8 bytes PCM data
        std::string wavData;
        {
            uint32_t dataSize = 8;
            uint32_t chunkSize = 36 + dataSize;
            uint16_t numChannels = 1;
            uint32_t sampleRate = 8000;
            uint16_t bitsPerSample = 16;
            uint32_t byteRate = sampleRate * numChannels * (bitsPerSample / 8);
            uint16_t blockAlign = numChannels * (bitsPerSample / 8);
            uint16_t audioFormat = 1;

            wavData.append("RIFF", 4);
            wavData.append(reinterpret_cast<const char*>(&chunkSize), 4);
            wavData.append("WAVE", 4);
            wavData.append("fmt ", 4);
            uint32_t sub1Size = 16;
            wavData.append(reinterpret_cast<const char*>(&sub1Size), 4);
            wavData.append(reinterpret_cast<const char*>(&audioFormat), 2);
            wavData.append(reinterpret_cast<const char*>(&numChannels), 2);
            wavData.append(reinterpret_cast<const char*>(&sampleRate), 4);
            wavData.append(reinterpret_cast<const char*>(&byteRate), 4);
            wavData.append(reinterpret_cast<const char*>(&blockAlign), 2);
            wavData.append(reinterpret_cast<const char*>(&bitsPerSample), 2);
            wavData.append("data", 4);
            wavData.append(reinterpret_cast<const char*>(&dataSize), 4);
            int16_t pcmSamples[4] = {0, 1000, -1000, 0};
            wavData.append(reinterpret_cast<const char*>(pcmSamples), sizeof(pcmSamples));
        }

        // 1. Upload valid WAV
        std::string boundary = "----TestAudioBoundaryXYZ123";
        std::string uploadBody =
            "--" + boundary + "\r\n"
            "Content-Disposition: form-data; name=\"title\"\r\n\r\n"
            "Uploaded Unit Track\r\n"
            "--" + boundary + "\r\n"
            "Content-Disposition: form-data; name=\"artist\"\r\n\r\n"
            "Unit Tester\r\n"
            "--" + boundary + "\r\n"
            "Content-Disposition: form-data; name=\"genre\"\r\n\r\n"
            "Synth\r\n"
            "--" + boundary + "\r\n"
            "Content-Disposition: form-data; name=\"file\"; filename=\"unit_test.wav\"\r\n"
            "Content-Type: audio/wav\r\n\r\n"
            + wavData + "\r\n"
            "--" + boundary + "--\r\n";

        httplib::Headers uploadHeaders = {
            {"Content-Type", "multipart/form-data; boundary=" + boundary}
        };
        auto uploadRes = cli.Post("/api/songs/upload", uploadHeaders, uploadBody, "multipart/form-data; boundary=" + boundary);
        TEST("Upload valid audio responds 201 Created", uploadRes && uploadRes->status == 201);
        int uploadedId = -1;
        if (uploadRes && uploadRes->status == 201) {
            auto j = nlohmann::json::parse(uploadRes->body);
            uploadedId = j["id"].get<int>();
            TEST("Uploaded song has ID >= 200", uploadedId >= 200);
            TEST("Uploaded song hasAudio is true", j["hasAudio"] == true);
            TEST("Uploaded song title matches", j["title"] == "Uploaded Unit Track");
        }

        // 2. Stream uploaded song
        if (uploadedId > 0) {
            std::string audioStreamPath = "/api/songs/" + std::to_string(uploadedId) + "/audio";
            auto streamUpRes = cli.Get(audioStreamPath.c_str());
            TEST("Stream uploaded audio responds 200", streamUpRes && streamUpRes->status == 200);
            if (streamUpRes && streamUpRes->status == 200) {
                TEST("Uploaded stream content length matches", streamUpRes->body.size() == wavData.size());
            }

            // 3. Delete uploaded song and verify cleanup
            std::string delPath = "/api/songs/" + std::to_string(uploadedId);
            auto delUpRes = cli.Delete(delPath.c_str());
            TEST("Delete uploaded song responds 200", delUpRes && delUpRes->status == 200);

            // Verify audio URL now returns 404
            auto postDelRes = cli.Get(audioStreamPath.c_str());
            TEST("Audio endpoint after deletion returns 404", postDelRes && postDelRes->status == 404);
        }

        // 4. Reject invalid audio file (corrupted or non-audio payload)
        std::string badUploadBody =
            "--" + boundary + "\r\n"
            "Content-Disposition: form-data; name=\"title\"\r\n\r\n"
            "Bad File Track\r\n"
            "--" + boundary + "\r\n"
            "Content-Disposition: form-data; name=\"file\"; filename=\"document.txt\"\r\n"
            "Content-Type: text/plain\r\n\r\n"
            "This is just a text string, not an audio file!\r\n"
            "--" + boundary + "--\r\n";

        auto badRes = cli.Post("/api/songs/upload", uploadHeaders, badUploadBody, "multipart/form-data; boundary=" + boundary);
        TEST("Upload corrupted/invalid file rejected with 400", badRes && badRes->status == 400);
        if (badRes && badRes->status == 400) {
            auto j = nlohmann::json::parse(badRes->body);
            TEST("Rejection explains unsupported/corrupted format", j.contains("error"));
        }
    }

    std::cout << "\n  ====================================================\n";
    std::cout << "  Results: " << passedTests << " passed, "
              << failedTests << " failed, "
              << totalTests << " total\n";
    std::cout << "  ====================================================\n\n";

    // Clean shutdown
    server.stop();
    if (serverThread.joinable()) {
        serverThread.join();
    }

    return (failedTests == 0) ? 0 : 1;
}
