/**
 * @file main.cpp
 * @brief Entry point for the Smart Playlist Synthesizer application.
 *
 * This file contains only the main() function, which creates an
 * Application object and calls run(). All business logic resides
 * in the Application class and its composed components.
 *
 * Project: Smart Playlist Synthesizer
 * Course:  SY B.Tech — Object-Oriented Programming
 * Team:    Akshay Kurapati, Atharv Mane, Manas Chaudhari,
 *          Piyush Lasurkar, Naman Kumar Rai, RupeshKumar Nirankari
 */

#include "Application.h"
#include <iostream>

int main() {
    try {
        Application app;
        app.run();
    } catch (const std::exception& e) {
        std::cerr << "\n  [FATAL ERROR] " << e.what() << "\n";
        return 1;
    } catch (...) {
        std::cerr << "\n  [FATAL ERROR] Unknown exception occurred.\n";
        return 1;
    }
    return 0;
}
