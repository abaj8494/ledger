#ifndef LEDGER_API_LEDGER_MANAGER_H
#define LEDGER_API_LEDGER_MANAGER_H

#include <string>
#include <vector>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <sstream>
#include <array>
#include <regex>
#include <algorithm>
#include <nlohmann/json.hpp>

namespace ledger_api {

using json = nlohmann::json;

struct Posting {
    std::string account;
    std::string amount;
    std::string comment;
};

struct Transaction {
    int startLine = -1;
    std::string date;
    bool cleared = false;
    bool pending = false;
    std::string payee;
    std::vector<Posting> postings;
};

class LedgerManager {
public:
    static LedgerManager& getInstance() {
        static LedgerManager instance;
        return instance;
    }

    // Run a ledger command and return the output
    std::string runLedgerCommand(const std::string& args);

    // Parse the ledger file content
    std::string parseLedgerFile();

    // Write content to the ledger file
    bool writeLedgerFile(const std::string& content);

    // Parse transactions from ledger content
    std::vector<Transaction> parseTransactions(const std::string& content);

    // Get a specific transaction by index
    Transaction getTransaction(const std::string& content, int index);

    // Format a transaction for writing to the ledger file
    std::string formatTransaction(const Transaction& transaction);

    // Get account summary
    json getAccountSummary();

    // Get all transactions
    json getAllTransactions(int limit = 0);

    // Get all accounts for autocomplete
    json getAllAccounts();

    // Get balance report
    json getBalanceReport();

    // Get register report
    json getRegisterReport();

    // Get budget report
    json getBudgetReport();

    // Get cleared report
    json getClearedReport();

    // Add a new transaction
    bool addTransaction(const json& transactionData);

    // Update a transaction
    bool updateTransaction(int index, const json& transactionData);

    // Delete a transaction
    bool deleteTransaction(int index);

private:
    LedgerManager() = default;
    ~LedgerManager() = default;
    LedgerManager(const LedgerManager&) = delete;
    LedgerManager& operator=(const LedgerManager&) = delete;

    // Helper function to execute a command and get its output
    std::string execCommand(const std::string& cmd);

    // Helper to serialize account data 
    json serializeAccountData(const std::string& ledgerOutput);

    // Helper to update static reports
    void updateReports();
};

} // namespace ledger_api

#endif // LEDGER_API_LEDGER_MANAGER_H 