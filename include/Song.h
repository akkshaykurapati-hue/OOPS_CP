#ifndef SONG_H
#define SONG_H

#include <string>
#include <iostream>
#include <iomanip>
#include <stdexcept>

/**
 * @class Song
 * @brief Represents an individual music track in the Smart Playlist Synthesizer.
 *
 * OOP Concepts Demonstrated:
 *   - Encapsulation: All attributes are private; access is through public getters.
 *   - Constructors: Parameterized constructor with input validation.
 *   - Const correctness: Getters and display methods are const.
 *   - Abstraction: External code interacts with Song through a clean public interface.
 */
class Song {
private:
    int id;                    // Unique track identifier
    std::string title;         // Song title
    std::string artist;        // Artist / band name
    std::string genre;         // Genre category
    int durationSeconds;       // Duration in seconds (must be > 0)
    std::string album;         // Album name (optional, defaults to "Single")
    std::string artworkUrl;    // Custom artwork URL or identifier
    bool hasAudio;             // Whether actual audio media is attached

public:
    // ── Constructors ──────────────────────────────────────────────────

    /// Default constructor (needed for certain STL operations)
    Song();

    /// Parameterized constructor with validation and optional album/artwork/audio
    /// @throws std::invalid_argument if any mandatory field is invalid
    Song(int id, const std::string& title, const std::string& artist,
         const std::string& genre, int durationSeconds,
         const std::string& album = "", const std::string& artworkUrl = "",
         bool hasAudio = false);

    // ── Getters (const — never modify internal state) ─────────────────

    int getId() const;
    std::string getTitle() const;
    std::string getArtist() const;
    std::string getGenre() const;
    int getDurationSeconds() const;
    std::string getAlbum() const;
    std::string getArtworkUrl() const;
    bool hasAudioFile() const;

    // ── Setters (allow updating song metadata) ───────────────────────

    void setTitle(const std::string& newTitle);
    void setArtist(const std::string& newArtist);
    void setAlbum(const std::string& newAlbum);
    void setGenre(const std::string& newGenre);
    void setDurationSeconds(int newDuration);
    void setArtworkUrl(const std::string& newArtworkUrl);
    void setHasAudioFile(bool audioPresent);

    // ── Display helpers ───────────────────────────────────────────────

    /// Returns duration formatted as "MM:SS"
    std::string getFormattedDuration() const;

    /// Prints a single-line summary: ID | Title | Artist | Genre | Duration
    void display() const;

    /// Prints a detailed, multi-line card for the track
    void displayDetailed() const;

    // ── Validation ────────────────────────────────────────────────────

    /// Static helper: validates fields before construction
    static void validate(int id, const std::string& title,
                         const std::string& artist, const std::string& genre,
                         int durationSeconds);

    // ── Comparison operators (useful for sorting / searching) ─────────

    bool operator==(const Song& other) const;
    bool operator!=(const Song& other) const;
};

#endif // SONG_H
