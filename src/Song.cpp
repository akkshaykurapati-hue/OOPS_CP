#include "Song.h"
#include <sstream>

// ══════════════════════════════════════════════════════════════════════
// Constructors
// ══════════════════════════════════════════════════════════════════════

Song::Song()
    : id(0), title(""), artist(""), genre(""), durationSeconds(0),
      album("Single"), artworkUrl(""), hasAudio(false)
{
}

Song::Song(int id, const std::string& title, const std::string& artist,
           const std::string& genre, int durationSeconds,
           const std::string& album, const std::string& artworkUrl,
           bool hasAudio)
    : id(id), title(title), artist(artist), genre(genre),
      durationSeconds(durationSeconds),
      album(album.empty() ? "Single" : album),
      artworkUrl(artworkUrl),
      hasAudio(hasAudio)
{
    // Validate all fields; throws std::invalid_argument on failure
    validate(id, title, artist, genre, durationSeconds);
}

// ══════════════════════════════════════════════════════════════════════
// Getters
// ══════════════════════════════════════════════════════════════════════

int Song::getId() const { return id; }
std::string Song::getTitle() const { return title; }
std::string Song::getArtist() const { return artist; }
std::string Song::getGenre() const { return genre; }
int Song::getDurationSeconds() const { return durationSeconds; }
std::string Song::getAlbum() const { return album; }
std::string Song::getArtworkUrl() const { return artworkUrl; }
bool Song::hasAudioFile() const { return hasAudio; }

// ══════════════════════════════════════════════════════════════════════
// Setters
// ══════════════════════════════════════════════════════════════════════

void Song::setTitle(const std::string& newTitle) {
    if (newTitle.empty()) throw std::invalid_argument("Song title cannot be empty.");
    title = newTitle;
}

void Song::setArtist(const std::string& newArtist) {
    if (newArtist.empty()) throw std::invalid_argument("Artist name cannot be empty.");
    artist = newArtist;
}

void Song::setAlbum(const std::string& newAlbum) {
    album = newAlbum.empty() ? "Single" : newAlbum;
}

void Song::setGenre(const std::string& newGenre) {
    if (newGenre.empty()) throw std::invalid_argument("Genre cannot be empty.");
    genre = newGenre;
}

void Song::setDurationSeconds(int newDuration) {
    if (newDuration <= 0) throw std::invalid_argument("Duration must be a positive number of seconds.");
    durationSeconds = newDuration;
}

void Song::setArtworkUrl(const std::string& newArtworkUrl) {
    artworkUrl = newArtworkUrl;
}

void Song::setHasAudioFile(bool audioPresent) {
    hasAudio = audioPresent;
}

// ══════════════════════════════════════════════════════════════════════
// Display helpers
// ══════════════════════════════════════════════════════════════════════

std::string Song::getFormattedDuration() const {
    int minutes = durationSeconds / 60;
    int seconds = durationSeconds % 60;
    std::ostringstream oss;
    oss << minutes << ":" << std::setfill('0') << std::setw(2) << seconds;
    return oss.str();
}

void Song::display() const {
    std::cout << "  " << std::left
              << std::setw(5)  << id
              << std::setw(30) << title
              << std::setw(22) << artist
              << std::setw(14) << genre
              << std::right << std::setw(6) << getFormattedDuration()
              << std::endl;
}

void Song::displayDetailed() const {
    std::cout << "  +------------------------------------------+\n"
              << "  |  Track ID   : " << id << "\n"
              << "  |  Title      : " << title << "\n"
              << "  |  Artist     : " << artist << "\n"
              << "  |  Genre      : " << genre << "\n"
              << "  |  Duration   : " << getFormattedDuration() << "\n"
              << "  +------------------------------------------+\n";
}

// ══════════════════════════════════════════════════════════════════════
// Validation
// ══════════════════════════════════════════════════════════════════════

void Song::validate(int id, const std::string& title,
                    const std::string& artist, const std::string& genre,
                    int durationSeconds) {
    if (id <= 0) {
        throw std::invalid_argument("Song ID must be a positive integer.");
    }
    if (title.empty()) {
        throw std::invalid_argument("Song title cannot be empty.");
    }
    if (artist.empty()) {
        throw std::invalid_argument("Artist name cannot be empty.");
    }
    if (genre.empty()) {
        throw std::invalid_argument("Genre cannot be empty.");
    }
    if (durationSeconds <= 0) {
        throw std::invalid_argument("Duration must be a positive number of seconds.");
    }
}

// ══════════════════════════════════════════════════════════════════════
// Comparison operators
// ══════════════════════════════════════════════════════════════════════

bool Song::operator==(const Song& other) const {
    return id == other.id;
}

bool Song::operator!=(const Song& other) const {
    return !(*this == other);
}
