#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include "MusicLibrary.h"
#include "PlaylistManager.h"
#include "MusicPlayer.h"
#include "PlaylistStorage.h"

#include <string>
#include <mutex>
#include <chrono>
#include <memory>
#include "httplib.h"
#include "nlohmann/json.hpp"

/**
 * @class WebServer
 * @brief HTTP REST API Server and Static Asset Host for Smart Playlist Synthesizer.
 *
 * Exposes existing C++ business logic (MusicLibrary, PlaylistManager, MusicPlayer,
 * PlaylistStorage) via JSON endpoints and serves the modern dark web dashboard.
 *
 * Thread-safety:
 *   All route handlers acquire a mutex lock before accessing shared state.
 *
 * Simulated Playback:
 *   Tracks elapsed seconds using std::chrono against track duration without
 *   requiring real audio hardware.
 */
class WebServer {
public:
    WebServer(int port = 8080, const std::string& host = "127.0.0.1", const std::string& webRoot = "./web");
    ~WebServer();

    /// Initialize routes and load initial sample data
    void init();

    /// Start listening (blocking call)
    bool start();

    /// Stop the server
    void stop();

    /// Get current server host and port
    std::string getHost() const;
    int getPort() const;

    // Public state accessors for automated testing
    MusicLibrary& getLibrary() { return library; }
    PlaylistManager& getPlaylistManager() { return playlistManager; }
    MusicPlayer& getPlayer() { return player; }
    PlaylistStorage& getStorage() { return storage; }

private:
    int port;
    std::string host;
    std::string webRoot;

    httplib::Server server;
    std::mutex stateMutex;

    // Core domain models (reused unchanged)
    MusicLibrary library;
    PlaylistManager playlistManager;
    MusicPlayer player;
    PlaylistStorage storage;

    // Simulated playback metadata
    int activePlaylistIndex;
    int simulatedElapsedSeconds;
    std::chrono::steady_clock::time_point playbackStartTime;

    // ── Internal Helpers ──────────────────────────────────────────────
    void setupRoutes();
    void updateSimulatedTimeLocked();
    void ensurePlaylistPointerLocked();

    // ── Audio Storage & Catalog Persistence ───────────────────────────
    std::string audioStorageDir;
    std::string libraryCatalogPath;
    std::string playlistsManifestPath;
    std::unordered_map<int, std::string> songAudioFiles; // songId -> internal filename in audioStorageDir

    void saveLibraryCatalogLocked();
    bool loadLibraryCatalogLocked();
    void savePlaylistsManifestLocked();
    bool loadPlaylistsManifestLocked();
    void generateSampleAudioFiles();
    std::string getSafeAudioFilePath(int songId) const;

    // ── JSON Serializers ──────────────────────────────────────────────
    static nlohmann::json songToJson(const Song& song);
    static nlohmann::json playlistToJsonSummary(int id, const Playlist& pl);
    static nlohmann::json playlistToJsonDetailed(int id, const Playlist& pl);
    nlohmann::json getPlayerStatusJsonLocked();

    // ── Route Handlers ────────────────────────────────────────────────
    void handleHealth(const httplib::Request& req, httplib::Response& res);
    void handleGetSongs(const httplib::Request& req, httplib::Response& res);
    void handleGetSongById(const httplib::Request& req, httplib::Response& res);
    void handleAddSong(const httplib::Request& req, httplib::Response& res);
    void handleUpdateSong(const httplib::Request& req, httplib::Response& res);
    void handleDeleteSong(const httplib::Request& req, httplib::Response& res);
    void handleUploadSong(const httplib::Request& req, httplib::Response& res);
    void handleStreamAudio(const httplib::Request& req, httplib::Response& res);

    void handleGetPlaylists(const httplib::Request& req, httplib::Response& res);
    void handleCreatePlaylist(const httplib::Request& req, httplib::Response& res);
    void handleGetPlaylistById(const httplib::Request& req, httplib::Response& res);
    void handleUpdatePlaylist(const httplib::Request& req, httplib::Response& res);
    void handleDeletePlaylist(const httplib::Request& req, httplib::Response& res);

    void handleAddSongToPlaylist(const httplib::Request& req, httplib::Response& res);
    void handleRemoveSongFromPlaylist(const httplib::Request& req, httplib::Response& res);
    void handleShufflePlaylist(const httplib::Request& req, httplib::Response& res);
    void handleSortPlaylist(const httplib::Request& req, httplib::Response& res);

    void handleSavePlaylist(const httplib::Request& req, httplib::Response& res);
    void handleLoadPlaylist(const httplib::Request& req, httplib::Response& res);

    void handlePlayerPlay(const httplib::Request& req, httplib::Response& res);
    void handlePlayerPause(const httplib::Request& req, httplib::Response& res);
    void handlePlayerResume(const httplib::Request& req, httplib::Response& res);
    void handlePlayerNext(const httplib::Request& req, httplib::Response& res);
    void handlePlayerPrevious(const httplib::Request& req, httplib::Response& res);
    void handlePlayerSeek(const httplib::Request& req, httplib::Response& res);
    void handlePlayerStatus(const httplib::Request& req, httplib::Response& res);
};

#endif // WEB_SERVER_H
