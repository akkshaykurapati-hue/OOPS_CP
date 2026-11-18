#include "WebServer.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <sstream>
#include <cmath>
#include <chrono>

namespace fs = std::filesystem;

// ── Audio File Validation & Metadata Helper ────────────────────────────

struct AudioValidationResult {
    bool valid = false;
    std::string error;
    std::string detectedFormat; // "mp3", "wav", "ogg", "flac"
    std::string mimeType;
    int detectedDurationSeconds = 0;
};

static AudioValidationResult validateAudioData(const std::string& data, const std::string& originalFilename) {
    AudioValidationResult res;
    res.valid = false;
    res.detectedDurationSeconds = 0;

    if (data.size() < 44) {
        res.error = "File is empty or too small to be a valid audio file (minimum 44 bytes required).";
        return res;
    }

    const size_t MAX_SIZE = 50 * 1024 * 1024; // 50 MB
    if (data.size() > MAX_SIZE) {
        res.error = "File exceeds maximum permitted upload limit (50 MB).";
        return res;
    }

    // Inspect file extension
    std::string ext = "";
    size_t dotPos = originalFilename.find_last_of('.');
    if (dotPos != std::string::npos) {
        ext = originalFilename.substr(dotPos);
        std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) { return std::tolower(c); });
    }

    const uint8_t* bytes = reinterpret_cast<const uint8_t*>(data.data());
    size_t len = data.size();

    // 1. WAV Detection: starts with "RIFF" and bytes 8..11 are "WAVE"
    if (len >= 44 && bytes[0] == 'R' && bytes[1] == 'I' && bytes[2] == 'F' && bytes[3] == 'F' &&
        bytes[8] == 'W' && bytes[9] == 'A' && bytes[10] == 'V' && bytes[11] == 'E') {
        res.detectedFormat = "wav";
        res.mimeType = "audio/wav";
        res.valid = true;

        // Parse WAV chunks to determine sample rate & byte rate for duration
        uint32_t byteRate = 0;
        uint32_t dataBytes = 0;
        for (size_t i = 12; i + 8 <= len; ) {
            std::string chunkId(reinterpret_cast<const char*>(bytes + i), 4);
            uint32_t chunkSize = 0;
            std::memcpy(&chunkSize, bytes + i + 4, 4);

            if (chunkId == "fmt " && i + 8 + 16 <= len) {
                std::memcpy(&byteRate, bytes + i + 8 + 8, 4);
            } else if (chunkId == "data") {
                dataBytes = chunkSize;
            }

            if (chunkSize == 0 || i + 8 + chunkSize > len) break;
            i += 8 + chunkSize;
        }

        if (byteRate > 0 && dataBytes > 0) {
            res.detectedDurationSeconds = static_cast<int>(dataBytes / byteRate);
        }
        return res;
    }

    // 2. MP3 Detection: ID3v2 header ("ID3") or MPEG Audio Sync Frame (0xFF, high bits 0xE0)
    bool isId3 = (len >= 10 && bytes[0] == 'I' && bytes[1] == 'D' && bytes[2] == '3');
    bool isMpegSync = (bytes[0] == 0xFF && (bytes[1] & 0xE0) == 0xE0);
    if (isId3 || isMpegSync || ext == ".mp3") {
        if (isId3 || isMpegSync) {
            res.detectedFormat = "mp3";
            res.mimeType = "audio/mpeg";
            res.valid = true;
            // Estimated duration based on average 192kbps (24000 bytes/sec)
            res.detectedDurationSeconds = static_cast<int>(len / 24000);
            if (res.detectedDurationSeconds <= 0) res.detectedDurationSeconds = 30;
            return res;
        }
    }

    // 3. OGG Detection: starts with "OggS"
    if (len >= 4 && bytes[0] == 'O' && bytes[1] == 'g' && bytes[2] == 'g' && bytes[3] == 'S') {
        res.detectedFormat = "ogg";
        res.mimeType = "audio/ogg";
        res.valid = true;
        res.detectedDurationSeconds = static_cast<int>(len / 20000);
        if (res.detectedDurationSeconds <= 0) res.detectedDurationSeconds = 30;
        return res;
    }

    // 4. FLAC Detection: starts with "fLaC"
    if (len >= 4 && bytes[0] == 'f' && bytes[1] == 'L' && bytes[2] == 'a' && bytes[3] == 'C') {
        res.detectedFormat = "flac";
        res.mimeType = "audio/flac";
        res.valid = true;
        res.detectedDurationSeconds = static_cast<int>(len / 100000);
        if (res.detectedDurationSeconds <= 0) res.detectedDurationSeconds = 30;
        return res;
    }

    res.error = "Unsupported or corrupted audio file. Supported formats: MP3, WAV, OGG, FLAC.";
    return res;
}

// ── Audio Synthesis for Initial Royalty-Free Sample Tracks ─────────────

static void writeStereoWav(const std::string& filepath, int sampleRate, double durationSec,
                           const std::function<std::pair<double, double>(double t)>& synth) {
    uint32_t numSamples = static_cast<uint32_t>(sampleRate * durationSec);
    uint32_t numChannels = 2;
    uint16_t bitsPerSample = 16;
    uint32_t byteRate = sampleRate * numChannels * (bitsPerSample / 8);
    uint16_t blockAlign = numChannels * (bitsPerSample / 8);
    uint32_t dataSize = numSamples * blockAlign;
    uint32_t chunkSize = 36 + dataSize;

    std::ofstream ofs(filepath, std::ios::binary);
    if (!ofs) return;

    // RIFF Chunk
    ofs.write("RIFF", 4);
    ofs.write(reinterpret_cast<const char*>(&chunkSize), 4);
    ofs.write("WAVE", 4);

    // fmt subchunk
    ofs.write("fmt ", 4);
    uint32_t sub1Size = 16;
    uint16_t audioFormat = 1; // PCM
    ofs.write(reinterpret_cast<const char*>(&sub1Size), 4);
    ofs.write(reinterpret_cast<const char*>(&audioFormat), 2);
    ofs.write(reinterpret_cast<const char*>(&numChannels), 2);
    ofs.write(reinterpret_cast<const char*>(&sampleRate), 4);
    ofs.write(reinterpret_cast<const char*>(&byteRate), 4);
    ofs.write(reinterpret_cast<const char*>(&blockAlign), 2);
    ofs.write(reinterpret_cast<const char*>(&bitsPerSample), 2);

    // data subchunk
    ofs.write("data", 4);
    ofs.write(reinterpret_cast<const char*>(&dataSize), 4);

    std::vector<int16_t> buffer;
    buffer.reserve(numSamples * 2);

    double fadeTime = 0.20; // 200ms smooth envelope fade in/out

    for (uint32_t i = 0; i < numSamples; ++i) {
        double t = static_cast<double>(i) / sampleRate;
        double env = 1.0;
        if (t < fadeTime) {
            env = t / fadeTime;
        } else if (t > durationSec - fadeTime) {
            env = std::max(0.0, (durationSec - t) / fadeTime);
        }

        auto [left, right] = synth(t);
        left = std::clamp(left * env, -1.0, 1.0);
        right = std::clamp(right * env, -1.0, 1.0);

        int16_t sLeft = static_cast<int16_t>(left * 28000.0);
        int16_t sRight = static_cast<int16_t>(right * 28000.0);

        buffer.push_back(sLeft);
        buffer.push_back(sRight);
    }

    ofs.write(reinterpret_cast<const char*>(buffer.data()), buffer.size() * sizeof(int16_t));
    ofs.close();
}

// ── WebServer Implementation ──────────────────────────────────────────

WebServer::WebServer(int port, const std::string& host, const std::string& webRoot)
    : port(port), host(host), webRoot(webRoot),
      activePlaylistIndex(-1),
      simulatedElapsedSeconds(0),
      playbackStartTime(std::chrono::steady_clock::now()),
      audioStorageDir("./data/audio"),
      libraryCatalogPath("./data/library_catalog.json"),
      playlistsManifestPath("./data/playlists_manifest.json")
{
}

WebServer::~WebServer() {
    stop();
}

void WebServer::init() {
    // 1. Ensure required data directories exist
    try {
        fs::create_directories("./data");
        fs::create_directories(audioStorageDir);
    } catch (...) {}

    // 2. Generate royalty-free synthesized audio files for default library tracks
    generateSampleAudioFiles();

    // 3. Attempt to restore persistent library catalog
    bool catalogLoaded = loadLibraryCatalogLocked();
    if (!catalogLoaded) {
        // Fallback: Populate default sample data
        library.loadSampleData();

        // Attach audio references to sample tracks
        for (const auto& song : library.getAllSongs()) {
            int id = song.getId();
            std::string fname = "sample_" + std::to_string(id) + ".wav";
            if (fs::exists(audioStorageDir + "/" + fname)) {
                songAudioFiles[id] = fname;
                Song updated(id, song.getTitle(), song.getArtist(), song.getGenre(),
                             song.getDurationSeconds(), "Synthesizer Vol. 1", "", true);
                library.updateSong(updated);
            }
        }
        saveLibraryCatalogLocked();
    }

    // 4. Attempt to restore playlists manifest
    bool playlistsLoaded = loadPlaylistsManifestLocked();
    if (!playlistsLoaded || playlistManager.getPlaylistCount() == 0) {
        // Pre-create starter playlists
        playlistManager.createPlaylist("Chill & Focus");
        if (playlistManager.getPlaylistCount() > 0) {
            Playlist& p1 = playlistManager.getPlaylist(0);
            const auto& all = library.getAllSongs();
            if (all.size() >= 4) {
                p1.addSong(all[0]);
                p1.addSong(all[1]);
                p1.addSong(all[3]);
                p1.addSong(all[9]);
            }
        }

        playlistManager.createPlaylist("Synthwave Nights");
        if (playlistManager.getPlaylistCount() > 1) {
            Playlist& p2 = playlistManager.getPlaylist(1);
            const auto& all = library.getAllSongs();
            if (all.size() >= 7) {
                p2.addSong(all[0]); // 101
                p2.addSong(all[3]); // 104
                p2.addSong(all[6]); // 107
                p2.addSong(all[8]); // 109
            }
        }
        savePlaylistsManifestLocked();
    }

    // Set active playlist to the first one and bind player
    if (playlistManager.getPlaylistCount() > 0) {
        activePlaylistIndex = 0;
        player.loadPlaylist(&playlistManager.getPlaylist(0));
    }

    // 5. Configure HTTP routes and static mounting
    setupRoutes();
}

bool WebServer::start() {
    std::cout << "\n  ======================================================\n";
    std::cout << "  Smart Playlist Synthesizer Web Application\n";
    std::cout << "  ======================================================\n";
    std::cout << "  Backend: C++17 REST API Server (cpp-httplib)\n";
    std::cout << "  Audio Engine: Real HTML5 Streaming (Byte-Range Supported)\n";
    std::cout << "  Listening on: http://" << host << ":" << port << "\n";
    std::cout << "  Serving static assets from: " << webRoot << "\n";
    std::cout << "  Audio storage root: " << audioStorageDir << "\n";
    std::cout << "  Press Ctrl+C to stop the server.\n";
    std::cout << "  ======================================================\n\n";

    return server.listen(host.c_str(), port);
}

void WebServer::stop() {
    if (server.is_running()) {
        server.stop();
    }
}

std::string WebServer::getHost() const { return host; }
int WebServer::getPort() const { return port; }

// ── Audio Generation for Default Tracks ────────────────────────────────

void WebServer::generateSampleAudioFiles() {
    const double pi = 3.14159265358979323846;
    const int sampleRate = 44100;
    const double dur = 7.0; // 7-second musical loop per sample track

    struct Preset {
        int id;
        std::function<std::pair<double, double>(double t)> synth;
    };

    std::vector<Preset> presets = {
        // 101: Midnight Drive (Electronic) - 128 BPM pulsing synth bass & harmonic lead
        {101, [pi](double t) {
            double bass = std::sin(2 * pi * 110.0 * t) * 0.45;
            double pulse = (std::sin(2 * pi * 4.26 * t) > 0.0) ? 1.0 : 0.3;
            double lead = std::sin(2 * pi * 440.0 * t) * 0.25 * pulse;
            double chord = (std::sin(2 * pi * 554.37 * t) + std::sin(2 * pi * 659.25 * t)) * 0.12;
            return std::make_pair(bass + lead + chord, bass + lead * 0.8 + chord * 1.2);
        }},
        // 102: Fading Echoes (Indie Pop) - Warm acoustic chord arpeggio
        {102, [pi](double t) {
            double tMod = std::fmod(t, 2.0);
            double note = (tMod < 0.5) ? 261.63 : ((tMod < 1.0) ? 329.63 : ((tMod < 1.5) ? 392.00 : 523.25));
            double decay = std::exp(-std::fmod(t, 0.5) * 4.0);
            double tone = std::sin(2 * pi * note * t) * decay * 0.5;
            double sub = std::sin(2 * pi * 130.81 * t) * 0.2;
            return std::make_pair(tone + sub, tone * 0.9 + sub);
        }},
        // 103: Thunder Road (Rock) - Driving power chords with rich harmonic warmth
        {103, [pi](double t) {
            double root = std::sin(2 * pi * 164.81 * t) * 0.35; // E3
            double fifth = std::sin(2 * pi * 246.94 * t) * 0.30; // B3
            double oct = std::sin(2 * pi * 329.63 * t) * 0.20;  // E4
            double drive = std::tanh((root + fifth + oct) * 1.8) * 0.55;
            return std::make_pair(drive, drive * 0.95);
        }},
        // 104: City Lights (Synthwave) - Retro 80s Dm analog synth pad
        {104, [pi](double t) {
            double pad1 = std::sin(2 * pi * 146.83 * t) * 0.3; // D3
            double pad2 = std::sin(2 * pi * 220.00 * t) * 0.25; // A3
            double pad3 = std::sin(2 * pi * 349.23 * t) * 0.2;  // F4
            double pad4 = std::sin(2 * pi * 440.00 * t) * 0.15; // A4
            double chorus = std::sin(2 * pi * 0.6 * t) * 0.1;
            return std::make_pair(pad1 + pad2 + pad3 + chorus, pad1 + pad2 + pad4 - chorus);
        }},
        // 105: Starfall (Pop) - Bright crystalline bell chimes
        {105, [pi](double t) {
            double step = std::fmod(t, 1.0);
            double freq = (step < 0.25) ? 523.25 : ((step < 0.5) ? 659.25 : ((step < 0.75) ? 783.99 : 1046.50));
            double env = std::exp(-std::fmod(step, 0.25) * 9.0);
            double chime = (std::sin(2 * pi * freq * t) + 0.3 * std::sin(2 * pi * freq * 2.0 * t)) * env * 0.45;
            return std::make_pair(chime * (0.8 + 0.2 * std::sin(2 * pi * 2.0 * t)),
                                  chime * (0.8 - 0.2 * std::sin(2 * pi * 2.0 * t)));
        }},
        // 106: Rust and Roses (Alternative) - Melancholic Dm-Bb chord strum
        {106, [pi](double t) {
            double beat = std::fmod(t, 3.5);
            double bass = (beat < 1.75) ? 146.83 : 116.54;
            double chord = (beat < 1.75) ? (std::sin(2 * pi * 220.0 * t) + std::sin(2 * pi * 261.63 * t))
                                         : (std::sin(2 * pi * 233.08 * t) + std::sin(2 * pi * 293.66 * t));
            double body = std::sin(2 * pi * bass * t) * 0.4 + chord * 0.2;
            return std::make_pair(body, body * 0.9);
        }},
        // 107: Digital Rain (Electronic) - Cyberpunk 16th-note arpeggiator
        {107, [pi](double t) {
            int step = static_cast<int>(t * 8.0) % 8;
            double freqs[] = {220.0, 261.63, 329.63, 392.0, 440.0, 392.0, 329.63, 261.63};
            double f = freqs[step];
            double subDecay = std::exp(-std::fmod(t, 0.125) * 12.0);
            double arp = std::sin(2 * pi * f * t) * subDecay * 0.45;
            double sub = std::sin(2 * pi * 110.0 * t) * 0.2;
            return std::make_pair(arp + sub, arp * 0.85 + sub);
        }},
        // 108: Broken Compass (Folk) - Gentle acoustic fingerpicking harmony
        {108, [pi](double t) {
            double c1 = std::sin(2 * pi * 196.00 * t) * 0.35; // G3
            double c2 = std::sin(2 * pi * 246.94 * t) * 0.25; // B3
            double c3 = std::sin(2 * pi * 293.66 * t) * 0.20; // D4
            double warm = (c1 + c2 + c3) * 0.7;
            return std::make_pair(warm, warm * 0.95);
        }},
        // 109: Neon Shadows (Synthwave) - Dark driving synthwave arpeggio
        {109, [pi](double t) {
            double b = std::sin(2 * pi * 110.0 * t) * 0.4;
            double filter = 0.5 + 0.5 * std::sin(2 * pi * 0.5 * t);
            double lead = std::sin(2 * pi * 329.63 * t) * 0.25 * filter;
            double out = b + lead;
            return std::make_pair(out, out * 0.9);
        }},
        // 110: Ocean Lullaby (Ambient) - Ethereal ambient pads with slow resonant filter sweep
        {110, [pi](double t) {
            double sweep = 0.5 + 0.5 * std::sin(2 * pi * 0.25 * t);
            double p1 = std::sin(2 * pi * 130.81 * t) * 0.35; // C3
            double p2 = std::sin(2 * pi * 196.00 * t) * 0.30; // G3
            double p3 = std::sin(2 * pi * 261.63 * t) * 0.20 * sweep; // C4
            double swell = (p1 + p2 + p3) * 0.65;
            return std::make_pair(swell, swell * (0.8 + 0.2 * sweep));
        }}
    };

    for (const auto& p : presets) {
        std::string fname = "sample_" + std::to_string(p.id) + ".wav";
        std::string fullPath = audioStorageDir + "/" + fname;
        if (!fs::exists(fullPath)) {
            writeStereoWav(fullPath, sampleRate, dur, p.synth);
        }
        songAudioFiles[p.id] = fname;
    }
}

// ── Catalog & Manifest Persistence ─────────────────────────────────────

void WebServer::saveLibraryCatalogLocked() {
    try {
        nlohmann::json jCatalog = nlohmann::json::array();
        for (const auto& song : library.getAllSongs()) {
            int id = song.getId();
            nlohmann::json item = {
                {"id", id},
                {"title", song.getTitle()},
                {"artist", song.getArtist()},
                {"album", song.getAlbum()},
                {"genre", song.getGenre()},
                {"duration", song.getDurationSeconds()},
                {"artworkUrl", song.getArtworkUrl()},
                {"hasAudio", song.hasAudioFile()}
            };
            if (songAudioFiles.count(id) > 0) {
                item["audioFilename"] = songAudioFiles[id];
            } else {
                item["audioFilename"] = "";
            }
            jCatalog.push_back(item);
        }

        std::ofstream ofs(libraryCatalogPath);
        if (ofs) {
            ofs << jCatalog.dump(2);
        }
    } catch (...) {}
}

bool WebServer::loadLibraryCatalogLocked() {
    if (!fs::exists(libraryCatalogPath)) return false;

    try {
        std::ifstream ifs(libraryCatalogPath);
        if (!ifs) return false;

        auto jCatalog = nlohmann::json::parse(ifs);
        if (!jCatalog.is_array() || jCatalog.empty()) return false;

        library.clear();
        songAudioFiles.clear();

        for (const auto& item : jCatalog) {
            int id = item.value("id", 0);
            std::string title = item.value("title", "");
            std::string artist = item.value("artist", "");
            std::string album = item.value("album", "Single");
            std::string genre = item.value("genre", "Unknown");
            int duration = item.value("duration", 180);
            std::string artwork = item.value("artworkUrl", "");
            bool hasAudio = item.value("hasAudio", false);
            std::string audioFilename = item.value("audioFilename", "");

            if (id > 0 && !title.empty() && !artist.empty() && duration > 0) {
                Song song(id, title, artist, genre, duration, album, artwork, hasAudio);
                library.addSong(song);
                if (!audioFilename.empty()) {
                    songAudioFiles[id] = audioFilename;
                }
            }
        }
        return (library.getSongCount() > 0);
    } catch (...) {
        return false;
    }
}

void WebServer::savePlaylistsManifestLocked() {
    try {
        nlohmann::json jPlaylists = nlohmann::json::array();
        for (int i = 0; i < playlistManager.getPlaylistCount(); ++i) {
            const Playlist& pl = playlistManager.getPlaylistConst(i);
            nlohmann::json trackIds = nlohmann::json::array();
            for (const auto& track : pl.getTracks()) {
                trackIds.push_back(track.getId());
            }

            jPlaylists.push_back({
                {"id", i},
                {"name", pl.getName()},
                {"trackIds", trackIds}
            });
        }

        std::ofstream ofs(playlistsManifestPath);
        if (ofs) {
            ofs << jPlaylists.dump(2);
        }
    } catch (...) {}
}

bool WebServer::loadPlaylistsManifestLocked() {
    if (!fs::exists(playlistsManifestPath)) return false;

    try {
        std::ifstream ifs(playlistsManifestPath);
        if (!ifs) return false;

        auto jPlaylists = nlohmann::json::parse(ifs);
        if (!jPlaylists.is_array() || jPlaylists.empty()) return false;

        playlistManager.clear();

        for (const auto& item : jPlaylists) {
            std::string name = item.value("name", "");
            if (name.empty()) continue;

            if (playlistManager.createPlaylist(name)) {
                int plIndex = playlistManager.getPlaylistCount() - 1;
                Playlist& pl = playlistManager.getPlaylist(plIndex);
                if (item.contains("trackIds") && item["trackIds"].is_array()) {
                    for (int trackId : item["trackIds"]) {
                        const Song* s = library.findById(trackId);
                        if (s != nullptr) {
                            pl.addSong(*s);
                        }
                    }
                }
            }
        }
        return (playlistManager.getPlaylistCount() > 0);
    } catch (...) {
        return false;
    }
}

std::string WebServer::getSafeAudioFilePath(int songId) const {
    auto it = songAudioFiles.find(songId);
    if (it == songAudioFiles.end()) return "";

    std::string filename = it->second;
    if (filename.empty()) return "";
    // Security check: strictly disallow path traversal components
    if (filename.find("..") != std::string::npos || filename.find('/') != std::string::npos || filename.find('\\') != std::string::npos) {
        return "";
    }

    std::string fullPath = audioStorageDir + "/" + filename;
    if (fs::exists(fullPath)) return fullPath;
    return "";
}

// ── JSON Serializers ──────────────────────────────────────────────────

nlohmann::json WebServer::songToJson(const Song& song) {
    bool hasAudio = song.hasAudioFile();
    std::string audioUrl = hasAudio ? ("/api/songs/" + std::to_string(song.getId()) + "/audio") : "";
    return {
        {"id", song.getId()},
        {"title", song.getTitle()},
        {"artist", song.getArtist()},
        {"album", song.getAlbum()},
        {"genre", song.getGenre()},
        {"duration", song.getDurationSeconds()},
        {"formattedDuration", song.getFormattedDuration()},
        {"artworkUrl", song.getArtworkUrl()},
        {"hasAudio", hasAudio},
        {"audioUrl", audioUrl}
    };
}

nlohmann::json WebServer::playlistToJsonSummary(int id, const Playlist& pl) {
    return {
        {"id", id},
        {"name", pl.getName()},
        {"trackCount", pl.getTrackCount()},
        {"totalDuration", pl.getTotalDuration()},
        {"formattedDuration", pl.getFormattedTotalDuration()}
    };
}

nlohmann::json WebServer::playlistToJsonDetailed(int id, const Playlist& pl) {
    nlohmann::json jTracks = nlohmann::json::array();
    for (const auto& song : pl.getTracks()) {
        jTracks.push_back(songToJson(song));
    }

    return {
        {"id", id},
        {"name", pl.getName()},
        {"trackCount", pl.getTrackCount()},
        {"totalDuration", pl.getTotalDuration()},
        {"formattedDuration", pl.getFormattedTotalDuration()},
        {"tracks", jTracks}
    };
}

void WebServer::ensurePlaylistPointerLocked() {
    if (activePlaylistIndex >= 0 && activePlaylistIndex < playlistManager.getPlaylistCount()) {
        // Refresh pointer in case vector reallocated
        player.loadPlaylist(&playlistManager.getPlaylist(activePlaylistIndex));
    }
}

void WebServer::updateSimulatedTimeLocked() {
    if (player.getState() == MusicPlayer::State::PLAYING) {
        auto now = std::chrono::steady_clock::now();
        int delta = static_cast<int>(std::chrono::duration_cast<std::chrono::seconds>(now - playbackStartTime).count());
        int currentElapsed = simulatedElapsedSeconds + delta;

        const Song* currentSong = player.getCurrentSong();
        if (currentSong != nullptr && currentSong->getDurationSeconds() > 0) {
            if (currentElapsed >= currentSong->getDurationSeconds()) {
                // Song simulation completed! Auto-advance to next song
                if (activePlaylistIndex >= 0 && activePlaylistIndex < playlistManager.getPlaylistCount()) {
                    const Playlist& pl = playlistManager.getPlaylist(activePlaylistIndex);
                    if (player.getCurrentTrackIndex() < pl.getTrackCount() - 1) {
                        player.next();
                        simulatedElapsedSeconds = 0;
                        playbackStartTime = std::chrono::steady_clock::now();
                    } else {
                        // Reached the end of playlist
                        simulatedElapsedSeconds = currentSong->getDurationSeconds();
                        player.pause();
                    }
                }
            }
        }
    }
}

nlohmann::json WebServer::getPlayerStatusJsonLocked() {
    updateSimulatedTimeLocked();

    int elapsed = simulatedElapsedSeconds;
    if (player.getState() == MusicPlayer::State::PLAYING) {
        auto now = std::chrono::steady_clock::now();
        elapsed += static_cast<int>(std::chrono::duration_cast<std::chrono::seconds>(now - playbackStartTime).count());
    }

    const Song* currentSong = player.getCurrentSong();
    int duration = (currentSong != nullptr) ? currentSong->getDurationSeconds() : 0;
    if (duration > 0 && elapsed > duration) {
        elapsed = duration;
    }

    int mm = elapsed / 60;
    int ss = elapsed % 60;
    std::ostringstream oss;
    oss << mm << ":" << (ss < 10 ? "0" : "") << ss;
    std::string formattedElapsed = oss.str();

    double progressPercent = (duration > 0) ? (static_cast<double>(elapsed) / duration * 100.0) : 0.0;

    std::string playlistName = "None";
    if (activePlaylistIndex >= 0 && activePlaylistIndex < playlistManager.getPlaylistCount()) {
        playlistName = playlistManager.getPlaylist(activePlaylistIndex).getName();
    }

    nlohmann::json status = {
        {"state", player.getStateString()},
        {"activePlaylistId", activePlaylistIndex},
        {"playlistName", playlistName},
        {"currentTrackIndex", player.getCurrentTrackIndex()},
        {"elapsedSeconds", elapsed},
        {"durationSeconds", duration},
        {"formattedElapsed", formattedElapsed},
        {"progressPercent", progressPercent},
        {"isSimulated", true}
    };

    if (currentSong != nullptr) {
        status["track"] = songToJson(*currentSong);
    } else {
        status["track"] = nullptr;
    }

    return status;
}

// ── Route Registration ────────────────────────────────────────────────

void WebServer::setupRoutes() {
    // 1. Static file serving from webRoot
    if (fs::exists(webRoot)) {
        server.set_mount_point("/", webRoot.c_str());
    }

    // CORS & Preflight handling
    server.set_pre_routing_handler([](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type, Range, Authorization");
        res.set_header("Access-Control-Expose-Headers", "Content-Range, Accept-Ranges, Content-Length");
        if (req.method == "OPTIONS") {
            res.status = 204;
            return httplib::Server::HandlerResponse::Handled;
        }
        return httplib::Server::HandlerResponse::Unhandled;
    });

    // 2. Health check
    server.Get("/api/health", [this](const httplib::Request& req, httplib::Response& res) {
        handleHealth(req, res);
    });

    // 3. Song Library Endpoints
    server.Get("/api/songs", [this](const httplib::Request& req, httplib::Response& res) {
        handleGetSongs(req, res);
    });

    server.Get(R"(/api/songs/(\d+))", [this](const httplib::Request& req, httplib::Response& res) {
        handleGetSongById(req, res);
    });

    server.Post("/api/songs", [this](const httplib::Request& req, httplib::Response& res) {
        handleAddSong(req, res);
    });

    server.Put(R"(/api/songs/(\d+))", [this](const httplib::Request& req, httplib::Response& res) {
        handleUpdateSong(req, res);
    });

    server.Delete(R"(/api/songs/(\d+))", [this](const httplib::Request& req, httplib::Response& res) {
        handleDeleteSong(req, res);
    });

    // Audio upload & streaming
    server.Post("/api/songs/upload", [this](const httplib::Request& req, httplib::Response& res) {
        handleUploadSong(req, res);
    });

    server.Get(R"(/api/songs/(\d+)/audio)", [this](const httplib::Request& req, httplib::Response& res) {
        handleStreamAudio(req, res);
    });

    // 4. Playlist Endpoints
    server.Get("/api/playlists", [this](const httplib::Request& req, httplib::Response& res) {
        handleGetPlaylists(req, res);
    });

    server.Post("/api/playlists", [this](const httplib::Request& req, httplib::Response& res) {
        handleCreatePlaylist(req, res);
    });

    server.Get(R"(/api/playlists/(\d+))", [this](const httplib::Request& req, httplib::Response& res) {
        handleGetPlaylistById(req, res);
    });

    server.Put(R"(/api/playlists/(\d+))", [this](const httplib::Request& req, httplib::Response& res) {
        handleUpdatePlaylist(req, res);
    });

    server.Delete(R"(/api/playlists/(\d+))", [this](const httplib::Request& req, httplib::Response& res) {
        handleDeletePlaylist(req, res);
    });

    server.Post(R"(/api/playlists/(\d+)/songs)", [this](const httplib::Request& req, httplib::Response& res) {
        handleAddSongToPlaylist(req, res);
    });

    server.Delete(R"(/api/playlists/(\d+)/songs/(\d+))", [this](const httplib::Request& req, httplib::Response& res) {
        handleRemoveSongFromPlaylist(req, res);
    });

    server.Post(R"(/api/playlists/(\d+)/shuffle)", [this](const httplib::Request& req, httplib::Response& res) {
        handleShufflePlaylist(req, res);
    });

    server.Post(R"(/api/playlists/(\d+)/sort)", [this](const httplib::Request& req, httplib::Response& res) {
        handleSortPlaylist(req, res);
    });

    server.Post(R"(/api/playlists/(\d+)/save)", [this](const httplib::Request& req, httplib::Response& res) {
        handleSavePlaylist(req, res);
    });

    server.Post("/api/playlists/load", [this](const httplib::Request& req, httplib::Response& res) {
        handleLoadPlaylist(req, res);
    });

    // 5. Player Controls & State Sync
    server.Get("/api/player/status", [this](const httplib::Request& req, httplib::Response& res) {
        handlePlayerStatus(req, res);
    });

    server.Post("/api/player/play", [this](const httplib::Request& req, httplib::Response& res) {
        handlePlayerPlay(req, res);
    });

    server.Post("/api/player/pause", [this](const httplib::Request& req, httplib::Response& res) {
        handlePlayerPause(req, res);
    });

    server.Post("/api/player/resume", [this](const httplib::Request& req, httplib::Response& res) {
        handlePlayerResume(req, res);
    });

    server.Post("/api/player/next", [this](const httplib::Request& req, httplib::Response& res) {
        handlePlayerNext(req, res);
    });

    server.Post("/api/player/previous", [this](const httplib::Request& req, httplib::Response& res) {
        handlePlayerPrevious(req, res);
    });

    server.Post("/api/player/seek", [this](const httplib::Request& req, httplib::Response& res) {
        handlePlayerSeek(req, res);
    });
}

// ── Handlers Implementation ───────────────────────────────────────────

void WebServer::handleHealth(const httplib::Request& /*req*/, httplib::Response& res) {
    std::lock_guard<std::mutex> lock(stateMutex);
    nlohmann::json j = {
        {"status", "ok"},
        {"app", "Smart Playlist Synthesizer"},
        {"version", "2.0"},
        {"audioEngine", "HTML5 Real Audio Streaming"},
        {"songsCount", library.getSongCount()},
        {"playlistsCount", playlistManager.getPlaylistCount()},
        {"audioFilesCount", songAudioFiles.size()},
        {"playerState", player.getStateString()}
    };
    res.set_content(j.dump(), "application/json");
}

void WebServer::handleGetSongs(const httplib::Request& req, httplib::Response& res) {
    std::lock_guard<std::mutex> lock(stateMutex);
    std::vector<Song> result;

    if (req.has_param("q")) {
        std::string query = req.get_param_value("q");
        auto titleMatches = library.searchByTitle(query);
        auto artistMatches = library.searchByArtist(query);
        auto genreMatches = library.searchByGenre(query);

        std::unordered_set<int> seen;
        for (const auto& s : titleMatches) {
            if (seen.insert(s.getId()).second) result.push_back(s);
        }
        for (const auto& s : artistMatches) {
            if (seen.insert(s.getId()).second) result.push_back(s);
        }
        for (const auto& s : genreMatches) {
            if (seen.insert(s.getId()).second) result.push_back(s);
        }
    } else {
        result = library.getAllSongs();
    }

    if (req.has_param("sortBy")) {
        std::string sortBy = req.get_param_value("sortBy");
        if (sortBy == "title") {
            std::sort(result.begin(), result.end(), [](const Song& a, const Song& b) {
                return a.getTitle() < b.getTitle();
            });
        } else if (sortBy == "artist") {
            std::sort(result.begin(), result.end(), [](const Song& a, const Song& b) {
                return a.getArtist() < b.getArtist();
            });
        } else if (sortBy == "duration") {
            std::sort(result.begin(), result.end(), [](const Song& a, const Song& b) {
                return a.getDurationSeconds() < b.getDurationSeconds();
            });
        }
    }

    nlohmann::json jSongs = nlohmann::json::array();
    for (const auto& s : result) {
        jSongs.push_back(songToJson(s));
    }
    res.set_content(jSongs.dump(), "application/json");
}

void WebServer::handleGetSongById(const httplib::Request& req, httplib::Response& res) {
    std::lock_guard<std::mutex> lock(stateMutex);
    try {
        int id = std::stoi(req.matches[1]);
        const Song* song = library.findById(id);
        if (song == nullptr) {
            res.status = 404;
            res.set_content(nlohmann::json({{"error", "Song ID " + std::to_string(id) + " not found."}}).dump(), "application/json");
            return;
        }
        res.status = 200;
        res.set_content(songToJson(*song).dump(), "application/json");
    } catch (const std::exception& e) {
        res.status = 400;
        res.set_content(nlohmann::json({{"error", e.what()}}).dump(), "application/json");
    }
}

void WebServer::handleAddSong(const httplib::Request& req, httplib::Response& res) {
    std::lock_guard<std::mutex> lock(stateMutex);
    try {
        auto body = nlohmann::json::parse(req.body);
        int id = body.value("id", 0);
        std::string title = body.value("title", "");
        std::string artist = body.value("artist", "");
        std::string genre = body.value("genre", "");
        int duration = body.value("duration", 0);
        std::string album = body.value("album", "Single");
        std::string artworkUrl = body.value("artworkUrl", "");

        if (id <= 0 || title.empty() || artist.empty() || genre.empty() || duration <= 0) {
            res.status = 400;
            res.set_content(nlohmann::json({{"error", "Invalid song details. ID and duration must be > 0, fields cannot be blank."}}).dump(), "application/json");
            return;
        }

        Song newSong(id, title, artist, genre, duration, album, artworkUrl, false);
        if (!library.addSong(newSong)) {
            res.status = 409;
            res.set_content(nlohmann::json({{"error", "Song ID " + std::to_string(id) + " already exists in library."}}).dump(), "application/json");
            return;
        }

        saveLibraryCatalogLocked();

        res.status = 201;
        res.set_content(songToJson(newSong).dump(), "application/json");
    } catch (const std::exception& e) {
        res.status = 400;
        res.set_content(nlohmann::json({{"error", std::string("Malformed JSON: ") + e.what()}}).dump(), "application/json");
    }
}

void WebServer::handleUpdateSong(const httplib::Request& req, httplib::Response& res) {
    std::lock_guard<std::mutex> lock(stateMutex);
    try {
        int id = std::stoi(req.matches[1]);
        const Song* existing = library.findById(id);
        if (existing == nullptr) {
            res.status = 404;
            res.set_content(nlohmann::json({{"error", "Song ID " + std::to_string(id) + " not found."}}).dump(), "application/json");
            return;
        }

        auto body = nlohmann::json::parse(req.body);
        std::string title = body.value("title", existing->getTitle());
        std::string artist = body.value("artist", existing->getArtist());
        std::string album = body.value("album", existing->getAlbum());
        std::string genre = body.value("genre", existing->getGenre());
        int duration = body.value("duration", existing->getDurationSeconds());
        std::string artwork = body.value("artworkUrl", existing->getArtworkUrl());

        if (title.empty() || artist.empty() || genre.empty() || duration <= 0) {
            res.status = 400;
            res.set_content(nlohmann::json({{"error", "Invalid song details. Fields cannot be empty, duration must be > 0."}}).dump(), "application/json");
            return;
        }

        Song updated(id, title, artist, genre, duration, album, artwork, existing->hasAudioFile());
        library.updateSong(updated);
        playlistManager.updateSongInAllPlaylists(updated);
        saveLibraryCatalogLocked();
        savePlaylistsManifestLocked();

        res.status = 200;
        res.set_content(songToJson(updated).dump(), "application/json");
    } catch (const std::exception& e) {
        res.status = 400;
        res.set_content(nlohmann::json({{"error", e.what()}}).dump(), "application/json");
    }
}

void WebServer::handleDeleteSong(const httplib::Request& req, httplib::Response& res) {
    std::lock_guard<std::mutex> lock(stateMutex);
    try {
        int id = std::stoi(req.matches[1]);
        if (!library.removeSong(id)) {
            res.status = 404;
            res.set_content(nlohmann::json({{"error", "Song not found in library."}}).dump(), "application/json");
            return;
        }

        // Remove from all playlists automatically
        playlistManager.removeSongFromAllPlaylists(id);

        // Delete underlying physical audio file safely if attached
        if (songAudioFiles.count(id) > 0) {
            std::string filename = songAudioFiles[id];
            if (!filename.empty() && filename.find("..") == std::string::npos) {
                std::error_code ec;
                fs::remove(audioStorageDir + "/" + filename, ec);
            }
            songAudioFiles.erase(id);
        }

        ensurePlaylistPointerLocked();
        saveLibraryCatalogLocked();
        savePlaylistsManifestLocked();

        res.status = 200;
        res.set_content(nlohmann::json({{"status", "deleted"}, {"id", id}}).dump(), "application/json");
    } catch (const std::exception& e) {
        res.status = 400;
        res.set_content(nlohmann::json({{"error", e.what()}}).dump(), "application/json");
    }
}

void WebServer::handleUploadSong(const httplib::Request& req, httplib::Response& res) {
    std::lock_guard<std::mutex> lock(stateMutex);
    try {
        if (!req.is_multipart_form_data()) {
            res.status = 400;
            res.set_content(nlohmann::json({{"error", "Expected multipart/form-data request."}}).dump(), "application/json");
            return;
        }

        if (!req.has_file("file") && !req.has_file("audio")) {
            res.status = 400;
            res.set_content(nlohmann::json({{"error", "No audio file provided in upload request."}}).dump(), "application/json");
            return;
        }

        auto file = req.has_file("file") ? req.get_file_value("file") : req.get_file_value("audio");

        // Validate audio file content, extension, and size
        auto valResult = validateAudioData(file.content, file.filename);
        if (!valResult.valid) {
            res.status = (file.content.size() > 50 * 1024 * 1024) ? 413 : 400;
            res.set_content(nlohmann::json({{"error", valResult.error}}).dump(), "application/json");
            return;
        }

        auto getField = [&](const std::string& key, const std::string& defVal = "") -> std::string {
            if (req.has_param(key)) {
                std::string v = req.get_param_value(key);
                if (!v.empty()) return v;
            }
            if (req.has_file(key)) {
                std::string v = req.get_file_value(key).content;
                if (!v.empty()) return v;
            }
            return defVal;
        };

        // Determine unique Song ID
        int songId = 0;
        std::string rawId = getField("id");
        if (!rawId.empty()) {
            try { songId = std::stoi(rawId); } catch (...) {}
        }

        if (songId <= 0) {
            // Auto-assign next available ID >= 200
            int maxId = 199;
            for (const auto& s : library.getAllSongs()) {
                if (s.getId() > maxId) maxId = s.getId();
            }
            songId = maxId + 1;
        } else if (library.containsSong(songId)) {
            res.status = 409;
            res.set_content(nlohmann::json({{"error", "Song ID " + std::to_string(songId) + " already exists."}}).dump(), "application/json");
            return;
        }

        // Title: fallback to filename cleaned up
        std::string title = getField("title");
        if (title.empty()) {
            std::string rawName = file.filename;
            size_t slash = rawName.find_last_of("/\\");
            if (slash != std::string::npos) rawName = rawName.substr(slash + 1);
            size_t dot = rawName.find_last_of('.');
            if (dot != std::string::npos) rawName = rawName.substr(0, dot);
            std::replace(rawName.begin(), rawName.end(), '_', ' ');
            title = rawName.empty() ? ("Track " + std::to_string(songId)) : rawName;
        }

        std::string artist = getField("artist", "Unknown Artist");
        std::string album = getField("album", "Uploaded Audio");
        std::string genre = getField("genre", "Music");
        std::string artwork = getField("artworkUrl", "");

        int duration = 0;
        std::string rawDur = getField("duration");
        if (!rawDur.empty()) {
            try { duration = std::stoi(rawDur); } catch (...) {}
        }
        if (duration <= 0) {
            duration = (valResult.detectedDurationSeconds > 0) ? valResult.detectedDurationSeconds : 180;
        }

        // Generate safe unique filename
        auto nowEpoch = std::chrono::system_clock::now().time_since_epoch().count();
        std::string safeFilename = "track_" + std::to_string(songId) + "_" + std::to_string(nowEpoch) + "." + valResult.detectedFormat;
        std::string fullPath = audioStorageDir + "/" + safeFilename;

        // Write file safely to server storage
        std::ofstream ofs(fullPath, std::ios::binary);
        if (!ofs) {
            res.status = 500;
            res.set_content(nlohmann::json({{"error", "Failed to write audio file to storage directory."}}).dump(), "application/json");
            return;
        }
        ofs.write(file.content.data(), file.content.size());
        ofs.close();

        // Add to domain library and audio registry
        Song newSong(songId, title, artist, genre, duration, album, artwork, true);
        if (!library.addSong(newSong)) {
            fs::remove(fullPath);
            res.status = 409;
            res.set_content(nlohmann::json({{"error", "Failed to add song to library."}}).dump(), "application/json");
            return;
        }

        songAudioFiles[songId] = safeFilename;
        saveLibraryCatalogLocked();

        res.status = 201;
        res.set_content(songToJson(newSong).dump(), "application/json");
    } catch (const std::exception& e) {
        res.status = 400;
        res.set_content(nlohmann::json({{"error", std::string("Upload failed: ") + e.what()}}).dump(), "application/json");
    }
}

void WebServer::handleStreamAudio(const httplib::Request& req, httplib::Response& res) {
    std::lock_guard<std::mutex> lock(stateMutex);
    try {
        int id = std::stoi(req.matches[1]);
        if (!library.containsSong(id) || songAudioFiles.count(id) == 0) {
            res.status = 404;
            res.set_content(nlohmann::json({{"error", "Audio track not found for song ID " + std::to_string(id)}}).dump(), "application/json");
            return;
        }

        std::string safePath = getSafeAudioFilePath(id);
        if (safePath.empty()) {
            res.status = 404;
            res.set_content(nlohmann::json({{"error", "Audio file missing from server storage."}}).dump(), "application/json");
            return;
        }

        std::string ext = fs::path(safePath).extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) { return std::tolower(c); });

        std::string mimeType = "audio/mpeg";
        if (ext == ".wav") mimeType = "audio/wav";
        else if (ext == ".ogg") mimeType = "audio/ogg";
        else if (ext == ".flac") mimeType = "audio/flac";

        // Enable browser seeking and buffering with byte ranges
        auto mm = std::make_shared<httplib::detail::mmap>(safePath.c_str());
        if (!mm->is_open()) {
            res.status = 404;
            res.set_content(nlohmann::json({{"error", "Audio file could not be opened."}}).dump(), "application/json");
            return;
        }

        res.set_header("Accept-Ranges", "bytes");
        res.set_header("Content-Disposition", "inline");
        res.set_header("Cache-Control", "public, max-age=3600");
        res.set_content_provider(
            mm->size(),
            mimeType,
            [mm](size_t offset, size_t length, httplib::DataSink &sink) -> bool {
                sink.write(mm->data() + offset, length);
                return true;
            }
        );
    } catch (const std::exception& e) {
        res.status = 400;
        res.set_content(nlohmann::json({{"error", e.what()}}).dump(), "application/json");
    }
}

void WebServer::handleGetPlaylists(const httplib::Request& /*req*/, httplib::Response& res) {
    std::lock_guard<std::mutex> lock(stateMutex);
    nlohmann::json arr = nlohmann::json::array();
    for (int i = 0; i < playlistManager.getPlaylistCount(); ++i) {
        arr.push_back(playlistToJsonSummary(i, playlistManager.getPlaylistConst(i)));
    }
    res.set_content(arr.dump(), "application/json");
}

void WebServer::handleCreatePlaylist(const httplib::Request& req, httplib::Response& res) {
    std::lock_guard<std::mutex> lock(stateMutex);
    try {
        auto body = nlohmann::json::parse(req.body);
        std::string name = body.value("name", "");
        if (name.empty()) {
            res.status = 400;
            res.set_content(nlohmann::json({{"error", "Playlist name cannot be empty."}}).dump(), "application/json");
            return;
        }
        if (playlistManager.nameExists(name)) {
            res.status = 409;
            res.set_content(nlohmann::json({{"error", "A playlist named '" + name + "' already exists."}}).dump(), "application/json");
            return;
        }

        if (playlistManager.createPlaylist(name)) {
            int newIndex = playlistManager.getPlaylistCount() - 1;
            ensurePlaylistPointerLocked();
            savePlaylistsManifestLocked();
            res.status = 201;
            res.set_content(playlistToJsonSummary(newIndex, playlistManager.getPlaylistConst(newIndex)).dump(), "application/json");
        } else {
            res.status = 500;
            res.set_content(nlohmann::json({{"error", "Failed to create playlist."}}).dump(), "application/json");
        }
    } catch (const std::exception& e) {
        res.status = 400;
        res.set_content(nlohmann::json({{"error", std::string("Malformed request: ") + e.what()}}).dump(), "application/json");
    }
}

void WebServer::handleGetPlaylistById(const httplib::Request& req, httplib::Response& res) {
    std::lock_guard<std::mutex> lock(stateMutex);
    try {
        int id = std::stoi(req.matches[1]);
        if (id < 0 || id >= playlistManager.getPlaylistCount()) {
            res.status = 404;
            res.set_content(nlohmann::json({{"error", "Playlist ID not found."}}).dump(), "application/json");
            return;
        }
        res.set_content(playlistToJsonDetailed(id, playlistManager.getPlaylistConst(id)).dump(), "application/json");
    } catch (const std::exception& e) {
        res.status = 400;
        res.set_content(nlohmann::json({{"error", e.what()}}).dump(), "application/json");
    }
}

void WebServer::handleUpdatePlaylist(const httplib::Request& req, httplib::Response& res) {
    std::lock_guard<std::mutex> lock(stateMutex);
    try {
        int id = std::stoi(req.matches[1]);
        if (id < 0 || id >= playlistManager.getPlaylistCount()) {
            res.status = 404;
            res.set_content(nlohmann::json({{"error", "Playlist ID not found."}}).dump(), "application/json");
            return;
        }

        auto body = nlohmann::json::parse(req.body);
        std::string newName = body.value("name", "");
        if (newName.empty()) {
            res.status = 400;
            res.set_content(nlohmann::json({{"error", "Playlist name cannot be empty."}}).dump(), "application/json");
            return;
        }

        if (!playlistManager.renamePlaylist(id, newName)) {
            res.status = 409;
            res.set_content(nlohmann::json({{"error", "Cannot rename: name already exists or invalid."}}).dump(), "application/json");
            return;
        }

        savePlaylistsManifestLocked();
        res.set_content(playlistToJsonSummary(id, playlistManager.getPlaylistConst(id)).dump(), "application/json");
    } catch (const std::exception& e) {
        res.status = 400;
        res.set_content(nlohmann::json({{"error", e.what()}}).dump(), "application/json");
    }
}

void WebServer::handleDeletePlaylist(const httplib::Request& req, httplib::Response& res) {
    std::lock_guard<std::mutex> lock(stateMutex);
    try {
        int id = std::stoi(req.matches[1]);
        if (id < 0 || id >= playlistManager.getPlaylistCount()) {
            res.status = 404;
            res.set_content(nlohmann::json({{"error", "Playlist ID not found."}}).dump(), "application/json");
            return;
        }

        bool wasActive = (id == activePlaylistIndex);
        if (!playlistManager.deletePlaylist(id)) {
            res.status = 500;
            res.set_content(nlohmann::json({{"error", "Failed to delete playlist."}}).dump(), "application/json");
            return;
        }

        if (wasActive) {
            player.loadPlaylist(nullptr);
            activePlaylistIndex = (playlistManager.getPlaylistCount() > 0) ? 0 : -1;
            if (activePlaylistIndex >= 0) {
                player.loadPlaylist(&playlistManager.getPlaylist(activePlaylistIndex));
            }
        } else if (activePlaylistIndex > id) {
            --activePlaylistIndex;
            ensurePlaylistPointerLocked();
        }

        savePlaylistsManifestLocked();
        res.set_content(nlohmann::json({{"status", "deleted"}, {"id", id}}).dump(), "application/json");
    } catch (const std::exception& e) {
        res.status = 400;
        res.set_content(nlohmann::json({{"error", e.what()}}).dump(), "application/json");
    }
}

void WebServer::handleAddSongToPlaylist(const httplib::Request& req, httplib::Response& res) {
    std::lock_guard<std::mutex> lock(stateMutex);
    try {
        int id = std::stoi(req.matches[1]);
        if (id < 0 || id >= playlistManager.getPlaylistCount()) {
            res.status = 404;
            res.set_content(nlohmann::json({{"error", "Playlist ID not found."}}).dump(), "application/json");
            return;
        }

        auto body = nlohmann::json::parse(req.body);
        int songId = body.value("songId", 0);
        const Song* songPtr = library.findById(songId);
        if (songPtr == nullptr) {
            res.status = 404;
            res.set_content(nlohmann::json({{"error", "Song ID " + std::to_string(songId) + " not found in library."}}).dump(), "application/json");
            return;
        }

        Playlist& pl = playlistManager.getPlaylist(id);

        // Core requirement: prevent duplicate songs via std::unordered_set<int> in Playlist
        if (!pl.addSong(*songPtr)) {
            res.status = 409;
            res.set_content(nlohmann::json({
                {"error", "Duplicate track rejected: '" + songPtr->getTitle() + "' (ID " + std::to_string(songId) + ") already exists in playlist '" + pl.getName() + "'."}
            }).dump(), "application/json");
            return;
        }

        savePlaylistsManifestLocked();
        res.status = 200;
        res.set_content(playlistToJsonDetailed(id, pl).dump(), "application/json");
    } catch (const std::exception& e) {
        res.status = 400;
        res.set_content(nlohmann::json({{"error", e.what()}}).dump(), "application/json");
    }
}

void WebServer::handleRemoveSongFromPlaylist(const httplib::Request& req, httplib::Response& res) {
    std::lock_guard<std::mutex> lock(stateMutex);
    try {
        int plId = std::stoi(req.matches[1]);
        int songId = std::stoi(req.matches[2]);

        if (plId < 0 || plId >= playlistManager.getPlaylistCount()) {
            res.status = 404;
            res.set_content(nlohmann::json({{"error", "Playlist ID not found."}}).dump(), "application/json");
            return;
        }

        Playlist& pl = playlistManager.getPlaylist(plId);
        if (!pl.removeSong(songId)) {
            res.status = 404;
            res.set_content(nlohmann::json({{"error", "Track ID not found in playlist."}}).dump(), "application/json");
            return;
        }

        if (activePlaylistIndex == plId) {
            ensurePlaylistPointerLocked();
        }

        savePlaylistsManifestLocked();
        res.status = 200;
        res.set_content(playlistToJsonDetailed(plId, pl).dump(), "application/json");
    } catch (const std::exception& e) {
        res.status = 400;
        res.set_content(nlohmann::json({{"error", e.what()}}).dump(), "application/json");
    }
}

void WebServer::handleShufflePlaylist(const httplib::Request& req, httplib::Response& res) {
    std::lock_guard<std::mutex> lock(stateMutex);
    try {
        int plId = std::stoi(req.matches[1]);
        if (plId < 0 || plId >= playlistManager.getPlaylistCount()) {
            res.status = 404;
            res.set_content(nlohmann::json({{"error", "Playlist ID not found."}}).dump(), "application/json");
            return;
        }

        Playlist& pl = playlistManager.getPlaylist(plId);
        pl.shuffle();

        if (activePlaylistIndex == plId) {
            ensurePlaylistPointerLocked();
        }

        savePlaylistsManifestLocked();
        res.status = 200;
        res.set_content(playlistToJsonDetailed(plId, pl).dump(), "application/json");
    } catch (const std::exception& e) {
        res.status = 400;
        res.set_content(nlohmann::json({{"error", e.what()}}).dump(), "application/json");
    }
}

void WebServer::handleSortPlaylist(const httplib::Request& req, httplib::Response& res) {
    std::lock_guard<std::mutex> lock(stateMutex);
    try {
        int plId = std::stoi(req.matches[1]);
        if (plId < 0 || plId >= playlistManager.getPlaylistCount()) {
            res.status = 404;
            res.set_content(nlohmann::json({{"error", "Playlist ID not found."}}).dump(), "application/json");
            return;
        }

        auto body = nlohmann::json::parse(req.body);
        std::string sortBy = body.value("sortBy", "title");

        Playlist& pl = playlistManager.getPlaylist(plId);
        if (sortBy == "artist") {
            pl.sortByArtist();
        } else if (sortBy == "duration") {
            pl.sortByDuration();
        } else {
            pl.sortByTitle();
        }

        if (activePlaylistIndex == plId) {
            ensurePlaylistPointerLocked();
        }

        savePlaylistsManifestLocked();
        res.status = 200;
        res.set_content(playlistToJsonDetailed(plId, pl).dump(), "application/json");
    } catch (const std::exception& e) {
        res.status = 400;
        res.set_content(nlohmann::json({{"error", e.what()}}).dump(), "application/json");
    }
}

void WebServer::handleSavePlaylist(const httplib::Request& req, httplib::Response& res) {
    std::lock_guard<std::mutex> lock(stateMutex);
    try {
        int plId = std::stoi(req.matches[1]);
        if (plId < 0 || plId >= playlistManager.getPlaylistCount()) {
            res.status = 404;
            res.set_content(nlohmann::json({{"error", "Playlist ID not found."}}).dump(), "application/json");
            return;
        }

        const Playlist& pl = playlistManager.getPlaylistConst(plId);
        std::string filename = "";
        if (!req.body.empty()) {
            try {
                auto body = nlohmann::json::parse(req.body);
                filename = body.value("filename", "");
            } catch (...) {}
        }
        if (filename.empty()) {
            filename = PlaylistStorage::generateFilename(pl.getName());
        }

        fs::create_directories(PlaylistStorage::getDefaultDirectory());
        std::string filepath = PlaylistStorage::getDefaultDirectory() + filename;

        if (!storage.savePlaylist(pl, filepath)) {
            res.status = 500;
            res.set_content(nlohmann::json({{"error", "Failed to write playlist file to disk."}}).dump(), "application/json");
            return;
        }

        res.status = 200;
        res.set_content(nlohmann::json({
            {"status", "saved"},
            {"filename", filename},
            {"filepath", filepath},
            {"playlistName", pl.getName()},
            {"trackCount", pl.getTrackCount()}
        }).dump(), "application/json");
    } catch (const std::exception& e) {
        res.status = 400;
        res.set_content(nlohmann::json({{"error", e.what()}}).dump(), "application/json");
    }
}

void WebServer::handleLoadPlaylist(const httplib::Request& req, httplib::Response& res) {
    std::lock_guard<std::mutex> lock(stateMutex);
    try {
        auto body = nlohmann::json::parse(req.body);
        std::string filename = body.value("filename", "");
        if (filename.empty()) {
            filename = body.value("filepath", "");
        }

        if (filename.empty()) {
            res.status = 400;
            res.set_content(nlohmann::json({{"error", "Filename or filepath must be provided."}}).dump(), "application/json");
            return;
        }

        if (filename.find("..") != std::string::npos) {
            res.status = 400;
            res.set_content(nlohmann::json({{"error", "Invalid file path (path traversal disallowed)."}}).dump(), "application/json");
            return;
        }

        std::string filepath = filename;
        if (!fs::exists(filepath)) {
            std::string alt = PlaylistStorage::getDefaultDirectory() + filename;
            if (fs::exists(alt)) {
                filepath = alt;
            } else {
                res.status = 404;
                res.set_content(nlohmann::json({{"error", "File not found: " + filename}}).dump(), "application/json");
                return;
            }
        }

        Playlist loadedPlaylist;
        if (!storage.loadPlaylist(filepath, loadedPlaylist)) {
            res.status = 400;
            res.set_content(nlohmann::json({{"error", "Could not parse or load playlist from " + filepath}}).dump(), "application/json");
            return;
        }

        std::string baseName = loadedPlaylist.getName();
        std::string uniqueName = baseName;
        int counter = 1;
        while (playlistManager.nameExists(uniqueName)) {
            uniqueName = baseName + " (" + std::to_string(counter++) + ")";
        }
        loadedPlaylist.setName(uniqueName);

        playlistManager.addPlaylist(loadedPlaylist);
        int newIndex = playlistManager.getPlaylistCount() - 1;

        ensurePlaylistPointerLocked();
        savePlaylistsManifestLocked();

        res.status = 200;
        res.set_content(playlistToJsonDetailed(newIndex, playlistManager.getPlaylistConst(newIndex)).dump(), "application/json");
    } catch (const std::exception& e) {
        res.status = 400;
        res.set_content(nlohmann::json({{"error", e.what()}}).dump(), "application/json");
    }
}

// ── Player Route Handlers & State Sync ────────────────────────────────

void WebServer::handlePlayerPlay(const httplib::Request& req, httplib::Response& res) {
    std::lock_guard<std::mutex> lock(stateMutex);
    try {
        int playlistId = activePlaylistIndex;
        int targetTrackIndex = -1;

        if (!req.body.empty()) {
            try {
                auto body = nlohmann::json::parse(req.body);
                if (body.contains("playlistId")) {
                    playlistId = body.value("playlistId", activePlaylistIndex);
                }
                if (body.contains("trackIndex")) {
                    targetTrackIndex = body.value("trackIndex", -1);
                }
            } catch (...) {}
        }

        if (playlistId < 0 || playlistId >= playlistManager.getPlaylistCount()) {
            res.status = 400;
            res.set_content(nlohmann::json({{"error", "No valid playlist selected."}}).dump(), "application/json");
            return;
        }

        if (activePlaylistIndex != playlistId || !player.hasPlaylist()) {
            activePlaylistIndex = playlistId;
            player.loadPlaylist(&playlistManager.getPlaylist(activePlaylistIndex));
        }

        if (targetTrackIndex >= 0) {
            player.play();
            while (player.getCurrentTrackIndex() < targetTrackIndex &&
                   player.getCurrentTrackIndex() < playlistManager.getPlaylistConst(activePlaylistIndex).getTrackCount() - 1) {
                player.next();
            }
            simulatedElapsedSeconds = 0;
            playbackStartTime = std::chrono::steady_clock::now();
        } else {
            if (player.getState() == MusicPlayer::State::PAUSED) {
                player.resume();
                playbackStartTime = std::chrono::steady_clock::now();
            } else if (player.getState() == MusicPlayer::State::STOPPED) {
                player.play();
                simulatedElapsedSeconds = 0;
                playbackStartTime = std::chrono::steady_clock::now();
            }
        }

        res.set_content(getPlayerStatusJsonLocked().dump(), "application/json");
    } catch (const std::exception& e) {
        res.status = 400;
        res.set_content(nlohmann::json({{"error", e.what()}}).dump(), "application/json");
    }
}

void WebServer::handlePlayerPause(const httplib::Request& /*req*/, httplib::Response& res) {
    std::lock_guard<std::mutex> lock(stateMutex);
    if (player.getState() == MusicPlayer::State::PLAYING) {
        auto now = std::chrono::steady_clock::now();
        simulatedElapsedSeconds += static_cast<int>(std::chrono::duration_cast<std::chrono::seconds>(now - playbackStartTime).count());
        player.pause();
    }
    res.set_content(getPlayerStatusJsonLocked().dump(), "application/json");
}

void WebServer::handlePlayerResume(const httplib::Request& /*req*/, httplib::Response& res) {
    std::lock_guard<std::mutex> lock(stateMutex);
    if (player.getState() == MusicPlayer::State::PAUSED) {
        player.resume();
        playbackStartTime = std::chrono::steady_clock::now();
    } else if (player.getState() == MusicPlayer::State::STOPPED && player.hasPlaylist()) {
        player.play();
        simulatedElapsedSeconds = 0;
        playbackStartTime = std::chrono::steady_clock::now();
    }
    res.set_content(getPlayerStatusJsonLocked().dump(), "application/json");
}

void WebServer::handlePlayerNext(const httplib::Request& /*req*/, httplib::Response& res) {
    std::lock_guard<std::mutex> lock(stateMutex);
    if (player.hasPlaylist()) {
        player.next();
        simulatedElapsedSeconds = 0;
        playbackStartTime = std::chrono::steady_clock::now();
    }
    res.set_content(getPlayerStatusJsonLocked().dump(), "application/json");
}

void WebServer::handlePlayerPrevious(const httplib::Request& /*req*/, httplib::Response& res) {
    std::lock_guard<std::mutex> lock(stateMutex);
    if (player.hasPlaylist()) {
        player.previous();
        simulatedElapsedSeconds = 0;
        playbackStartTime = std::chrono::steady_clock::now();
    }
    res.set_content(getPlayerStatusJsonLocked().dump(), "application/json");
}

void WebServer::handlePlayerSeek(const httplib::Request& req, httplib::Response& res) {
    std::lock_guard<std::mutex> lock(stateMutex);
    try {
        auto body = nlohmann::json::parse(req.body);
        int targetSeconds = body.value("seconds", 0);
        const Song* currentSong = player.getCurrentSong();
        if (currentSong != nullptr) {
            targetSeconds = std::max(0, std::min(targetSeconds, currentSong->getDurationSeconds()));
            simulatedElapsedSeconds = targetSeconds;
            playbackStartTime = std::chrono::steady_clock::now();
        }
        res.set_content(getPlayerStatusJsonLocked().dump(), "application/json");
    } catch (const std::exception& e) {
        res.status = 400;
        res.set_content(nlohmann::json({{"error", e.what()}}).dump(), "application/json");
    }
}

void WebServer::handlePlayerStatus(const httplib::Request& /*req*/, httplib::Response& res) {
    std::lock_guard<std::mutex> lock(stateMutex);
    res.set_content(getPlayerStatusJsonLocked().dump(), "application/json");
}
