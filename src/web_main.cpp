/**
 * @file web_main.cpp
 * @brief Entry point for the Smart Playlist Synthesizer Web Application backend.
 *
 * Runs the C++17 REST API server and hosts the frontend UI dashboard.
 * The original console application entry point (src/main.cpp) remains untouched.
 *
 * Course:  SY B.Tech — Object-Oriented Programming
 * Team:    Akshay Kurapati, Atharv Mane, Manas Chaudhari,
 *          Piyush Lasurkar, Naman Kumar Rai, RupeshKumar Nirankari
 */

#include "WebServer.h"
#include <iostream>
#include <csignal>
#include <cstdlib>
#include <string>

static WebServer* g_server = nullptr;

void signalHandler(int signum) {
    std::cout << "\n  [!] Received signal " << signum << ". Shutting down server gracefully...\n";
    if (g_server != nullptr) {
        g_server->stop();
    }
}

int main(int argc, char* argv[]) {
    int port = 8080;
    std::string host = "127.0.0.1";
    std::string webRoot = "./web";

    // Parse command line arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--port" && i + 1 < argc) {
            port = std::atoi(argv[++i]);
        } else if (arg == "--host" && i + 1 < argc) {
            host = argv[++i];
        } else if (arg == "--web-root" && i + 1 < argc) {
            webRoot = argv[++i];
        } else if (arg == "--help" || arg == "-h") {
            std::cout << "Usage: SmartPlaylistSynthesizerWeb [options]\n"
                      << "Options:\n"
                      << "  --port <port>       Port to listen on (default: 8080)\n"
                      << "  --host <ip>         Host address (default: 127.0.0.1)\n"
                      << "  --web-root <dir>    Directory containing web UI assets (default: ./web)\n"
                      << "  --help, -h          Show this help message\n";
            return 0;
        }
    }

    // Check environment variable PORT if set
    const char* envPort = std::getenv("PORT");
    if (envPort != nullptr) {
        port = std::atoi(envPort);
    }

    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    try {
        WebServer server(port, host, webRoot);
        g_server = &server;
        server.init();

        std::cout << "  Starting server at http://" << host << ":" << port << "/ ...\n";
        server.start();

        std::cout << "  Server stopped.\n";
    } catch (const std::exception& e) {
        std::cerr << "\n  [FATAL ERROR] " << e.what() << "\n";
        return 1;
    } catch (...) {
        std::cerr << "\n  [FATAL ERROR] Unknown exception occurred.\n";
        return 1;
    }

    return 0;
}
