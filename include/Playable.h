#ifndef PLAYABLE_H
#define PLAYABLE_H

/**
 * @class Playable
 * @brief Abstract interface defining playback operations.
 *
 * OOP Concepts Demonstrated:
 *   - Abstraction: Pure virtual functions define a contract without implementation.
 *   - Polymorphism: Derived classes (MusicPlayer) override these methods.
 *   - Inheritance: MusicPlayer inherits from Playable.
 *
 * This interface ensures that any class claiming to be "playable" must
 * implement play, pause, resume, next, and previous operations.
 * The virtual destructor enables safe polymorphic deletion.
 */
class Playable {
public:
    /// Virtual destructor for safe polymorphic destruction
    virtual ~Playable() {}

    /// Start or resume playback of the current track
    virtual void play() = 0;

    /// Pause playback
    virtual void pause() = 0;

    /// Resume playback after pause
    virtual void resume() = 0;

    /// Advance to the next track
    virtual void next() = 0;

    /// Go back to the previous track
    virtual void previous() = 0;

    /// Display current playback status
    virtual void showStatus() const = 0;
};

#endif // PLAYABLE_H
