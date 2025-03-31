#ifndef LEDGER_API_CONFIG_H
#define LEDGER_API_CONFIG_H

#include <string>
#include <fstream>
#include <stdexcept>
#include <nlohmann/json.hpp>

namespace ledger_api {

class Config {
public:
    static Config& getInstance() {
        static Config instance;
        return instance;
    }

    void load(const std::string& config_file) {
        try {
            std::ifstream file(config_file);
            if (!file.is_open()) {
                throw std::runtime_error("Could not open config file: " + config_file);
            }
            
            file >> config_data;
            
            port = config_data.value("port", 3001);
            ledger_file = config_data.value("ledger_file", "/var/www/ledger/data/demo.ledger");
            ledger_cmd = config_data.value("ledger_cmd", "ledger");
            update_reports_script = config_data.value("update_reports_script", "/var/www/ledger/update-reports.sh");
        } catch (const std::exception& e) {
            throw std::runtime_error("Error parsing config file: " + std::string(e.what()));
        }
    }

    int getPort() const { return port; }
    const std::string& getLedgerFile() const { return ledger_file; }
    const std::string& getLedgerCmd() const { return ledger_cmd; }
    const std::string& getUpdateReportsScript() const { return update_reports_script; }

private:
    Config() = default;
    ~Config() = default;
    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;

    nlohmann::json config_data;
    int port = 3001;
    std::string ledger_file = "/var/www/ledger/data/demo.ledger";
    std::string ledger_cmd = "ledger";
    std::string update_reports_script = "/var/www/ledger/update-reports.sh";
};

} // namespace ledger_api

#endif // LEDGER_API_CONFIG_H 