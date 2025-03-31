#include <iostream>
#include <string>
#include <crow.h>
#include "config.h"
#include "api_handler.h"

int main(int argc, char* argv[]) {
    std::string config_file = "ledger_api_config.json";
    if (argc > 1) {
        config_file = argv[1];
    }

    try {
        // Load configuration
        auto& config = ledger_api::Config::getInstance();
        config.load(config_file);

        // Initialize API handler
        ledger_api::ApiHandler apiHandler;

        // Create Crow app
        crow::SimpleApp app;

        // Initialize routes
        apiHandler.initRoutes(app);

        // Start the server
        std::cout << "Starting ledger API server on port " << config.getPort() << std::endl;
        app.port(config.getPort()).multithreaded().run();

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
} 