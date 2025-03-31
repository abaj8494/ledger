#include "ledger_manager.h"
#include "config.h"
#include <filesystem>
#include <boost/algorithm/string.hpp>

namespace ledger_api {

std::string LedgerManager::execCommand(const std::string& cmd) {
    std::array<char, 128> buffer;
    std::string result;
    
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    
    return result;
}

std::string LedgerManager::runLedgerCommand(const std::string& args) {
    const auto& config = Config::getInstance();
    std::string cmd = config.getLedgerCmd() + " -f " + config.getLedgerFile() + " " + args;
    
    try {
        return execCommand(cmd);
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to execute ledger command: " + std::string(e.what()));
    }
}

std::string LedgerManager::parseLedgerFile() {
    const auto& config = Config::getInstance();
    std::ifstream file(config.getLedgerFile());
    
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open ledger file: " + config.getLedgerFile());
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

bool LedgerManager::writeLedgerFile(const std::string& content) {
    const auto& config = Config::getInstance();
    
    try {
        // Create a backup first
        std::filesystem::path ledgerFile(config.getLedgerFile());
        std::filesystem::path backupFile = ledgerFile.string() + ".bak";
        std::filesystem::copy_file(ledgerFile, backupFile, std::filesystem::copy_options::overwrite_existing);
        
        // Write the new content
        std::ofstream file(config.getLedgerFile());
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open ledger file for writing: " + config.getLedgerFile());
        }
        
        file << content;
        file.close();
        
        return true;
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to write to ledger file: " + std::string(e.what()));
    }
}

std::vector<Transaction> LedgerManager::parseTransactions(const std::string& content) {
    std::vector<Transaction> transactions;
    Transaction currentTransaction;
    
    std::istringstream stream(content);
    std::string line;
    int lineNum = 0;
    
    std::regex transactionRegex(R"(^(\d{4}/\d{2}/\d{2})\s+(\*|\!)?\s*(.*))");
    
    while (std::getline(stream, line)) {
        std::string trimmedLine = line;
        boost::trim(trimmedLine);
        
        if (trimmedLine.empty()) {
            lineNum++;
            continue;
        }
        
        std::smatch matches;
        
        // Check if this is a transaction line (begins with a date)
        if (std::regex_match(trimmedLine, matches, transactionRegex)) {
            // If we already have a transaction, add it to the vector
            if (currentTransaction.startLine != -1) {
                transactions.push_back(currentTransaction);
            }
            
            // Start a new transaction
            currentTransaction = Transaction();
            currentTransaction.startLine = lineNum;
            currentTransaction.date = matches[1];
            currentTransaction.cleared = (matches[2] == "*");
            currentTransaction.pending = (matches[2] == "!");
            currentTransaction.payee = matches[3];
        } 
        // If line begins with whitespace, it's a posting or comment
        else if (!trimmedLine.empty() && std::isspace(line[0])) {
            // Skip if no current transaction
            if (currentTransaction.startLine == -1) {
                lineNum++;
                continue;
            }
            
            // If line starts with a semicolon, it's a comment - skip it
            if (trimmedLine[0] == ';') {
                lineNum++;
                continue;
            }
            
            // It's a posting - extract account and amount
            std::string account = trimmedLine;
            std::string amount = "";
            
            try {
                // Find where the amount might start (look for multiple spaces or dollar sign)
                size_t dollarPos = trimmedLine.find('$');
                size_t spacePos = 0;
                
                // Find a sequence of 2 or more spaces
                for (size_t i = 0; i < trimmedLine.length() - 1; i++) {
                    if (std::isspace(trimmedLine[i]) && std::isspace(trimmedLine[i+1])) {
                        spacePos = i;
                        break;
                    }
                }
                
                // Determine where to split account and amount
                size_t splitPos = std::string::npos;
                if (dollarPos != std::string::npos && (spacePos == 0 || dollarPos < spacePos)) {
                    splitPos = dollarPos;
                    // Include the dollar sign with the amount
                } else if (spacePos != 0) {
                    splitPos = spacePos;
                }
                
                // If we found a delimiter, split the posting
                if (splitPos != std::string::npos) {
                    account = trimmedLine.substr(0, splitPos);
                    boost::trim(account);
                    
                    // Extract amount, handling potential comment
                    std::string rest = trimmedLine.substr(splitPos);
                    size_t commentPos = rest.find(';');
                    if (commentPos != std::string::npos) {
                        amount = rest.substr(0, commentPos);
                    } else {
                        amount = rest;
                    }
                    boost::trim(amount);
                }
            } catch (const std::exception& e) {
                std::cerr << "Error parsing posting line: " << trimmedLine << std::endl;
                std::cerr << "Error: " << e.what() << std::endl;
                // Just use the whole line as the account
                account = trimmedLine;
                amount = "";
            }
            
            // Add the posting to the current transaction
            Posting posting;
            posting.account = account;
            posting.amount = amount;
            currentTransaction.postings.push_back(posting);
        }
        
        lineNum++;
    }
    
    // Don't forget to add the last transaction
    if (currentTransaction.startLine != -1) {
        transactions.push_back(currentTransaction);
    }
    
    return transactions;
}

Transaction LedgerManager::getTransaction(const std::string& content, int index) {
    auto transactions = parseTransactions(content);
    
    if (index < 0 || index >= static_cast<int>(transactions.size())) {
        throw std::out_of_range("Transaction index out of range");
    }
    
    return transactions[index];
}

std::string LedgerManager::formatTransaction(const Transaction& transaction) {
    std::string result = transaction.date + " " + 
                        (transaction.cleared ? "* " : (transaction.pending ? "! " : "")) + 
                        transaction.payee + "\n";
    
    for (const auto& posting : transaction.postings) {
        result += "  " + posting.account;
        
        if (!posting.amount.empty()) {
            // Ensure there are at least two spaces between account and amount
            size_t amountPadding = std::max<size_t>(2, 50 - posting.account.length());
            result += std::string(amountPadding, ' ') + posting.amount;
        }
        
        if (!posting.comment.empty()) {
            result += "  ; " + posting.comment;
        }
        
        result += "\n";
    }
    
    return result;
}

json LedgerManager::serializeAccountData(const std::string& ledgerOutput) {
    json result = json::array();
    std::istringstream stream(ledgerOutput);
    std::string line;
    
    while (std::getline(stream, line)) {
        boost::trim(line);
        if (line.empty()) continue;
        
        // Calculate indentation level
        size_t firstNonSpace = line.find_first_not_of(" ");
        int level = firstNonSpace > 0 ? static_cast<int>(firstNonSpace) / 2 : 0;
        
        // Split the line by whitespace (2 or more spaces)
        std::vector<std::string> parts;
        boost::split(parts, line, boost::is_any_of("  "), boost::token_compress_on);
        
        if (parts.size() >= 2) {
            // Clean up any remaining whitespace
            for (auto& part : parts) {
                boost::trim(part);
            }
            
            json item;
            item["amount"] = parts[0];
            item["account"] = parts[1];
            item["level"] = level;
            
            result.push_back(item);
        }
    }
    
    return result;
}

json LedgerManager::getAccountSummary() {
    std::string output = runLedgerCommand("balance ^Assets ^Liabilities --depth 2");
    return serializeAccountData(output);
}

json LedgerManager::getAllTransactions(int limit) {
    std::string content = parseLedgerFile();
    auto transactions = parseTransactions(content);
    
    // Debug output to verify parsing
    std::ofstream debugLog("/tmp/ledger_debug.log");
    if (debugLog.is_open()) {
        debugLog << "Parsed " << transactions.size() << " transactions\n";
        for (const auto& transaction : transactions) {
            debugLog << "Transaction: " << transaction.date << " " 
                    << (transaction.cleared ? "*" : "") 
                    << " " << transaction.payee << "\n";
            
            for (const auto& posting : transaction.postings) {
                debugLog << "  Posting: account='" << posting.account << "', amount='" 
                        << posting.amount << "', comment='" << posting.comment << "'\n";
            }
            debugLog << "\n";
        }
        debugLog.close();
    }
    
    // Reverse the vector to get most recent first
    std::reverse(transactions.begin(), transactions.end());
    
    // Apply limit if specified
    if (limit > 0 && limit < static_cast<int>(transactions.size())) {
        transactions.resize(limit);
    }
    
    // Convert to JSON
    json result = json::array();
    
    for (const auto& transaction : transactions) {
        json jsonTransaction;
        jsonTransaction["date"] = transaction.date;
        jsonTransaction["cleared"] = transaction.cleared;
        jsonTransaction["pending"] = transaction.pending;
        jsonTransaction["payee"] = transaction.payee;
        
        json jsonPostings = json::array();
        for (const auto& posting : transaction.postings) {
            json jsonPosting;
            jsonPosting["account"] = posting.account;
            jsonPosting["amount"] = posting.amount;
            jsonPosting["comment"] = posting.comment;
            jsonPostings.push_back(jsonPosting);
        }
        
        jsonTransaction["postings"] = jsonPostings;
        result.push_back(jsonTransaction);
    }
    
    return result;
}

json LedgerManager::getAllAccounts() {
    std::string output = runLedgerCommand("accounts");
    
    json result = json::array();
    std::istringstream stream(output);
    std::string line;
    
    while (std::getline(stream, line)) {
        boost::trim(line);
        if (!line.empty()) {
            result.push_back(line);
        }
    }
    
    return result;
}

json LedgerManager::getBalanceReport() {
    std::string output = runLedgerCommand("balance");
    return serializeAccountData(output);
}

json LedgerManager::getRegisterReport() {
    std::string output = runLedgerCommand("register");
    
    json result = json::array();
    std::istringstream stream(output);
    std::string line;
    
    while (std::getline(stream, line)) {
        boost::trim(line);
        if (line.empty()) continue;
        
        std::vector<std::string> parts;
        boost::split(parts, line, boost::is_any_of("  "), boost::token_compress_on);
        
        if (parts.size() >= 5) {
            // Clean up parts
            for (auto& part : parts) {
                boost::trim(part);
            }
            
            json entry;
            entry["date"] = parts[0];
            entry["payee"] = parts[1];
            entry["account"] = parts[2];
            entry["amount"] = parts[3];
            entry["balance"] = parts[4];
            
            result.push_back(entry);
        }
    }
    
    return result;
}

json LedgerManager::getBudgetReport() {
    std::string output = runLedgerCommand("balance ^Expenses --budget");
    
    json result = json::array();
    std::istringstream stream(output);
    std::string line;
    
    while (std::getline(stream, line)) {
        boost::trim(line);
        if (line.empty()) continue;
        
        // Calculate indentation level
        size_t firstNonSpace = line.find_first_not_of(" ");
        int level = firstNonSpace > 0 ? static_cast<int>(firstNonSpace) / 2 : 0;
        
        std::vector<std::string> parts;
        boost::split(parts, line, boost::is_any_of("  "), boost::token_compress_on);
        
        if (parts.size() >= 4) {
            // Clean up parts
            for (auto& part : parts) {
                boost::trim(part);
            }
            
            json entry;
            entry["actual"] = parts[0];
            entry["budget"] = parts[1];
            entry["remaining"] = parts[2];
            entry["percent"] = parts[3];
            entry["account"] = parts.size() > 4 ? parts[4] : "";
            entry["level"] = level;
            
            result.push_back(entry);
        }
    }
    
    return result;
}

json LedgerManager::getClearedReport() {
    std::string output = runLedgerCommand("balance --cleared --pending");
    
    json result = json::array();
    std::istringstream stream(output);
    std::string line;
    
    while (std::getline(stream, line)) {
        boost::trim(line);
        if (line.empty()) continue;
        
        // Calculate indentation level
        size_t firstNonSpace = line.find_first_not_of(" ");
        int level = firstNonSpace > 0 ? static_cast<int>(firstNonSpace) / 2 : 0;
        
        std::vector<std::string> parts;
        boost::split(parts, line, boost::is_any_of("  "), boost::token_compress_on);
        
        if (parts.size() >= 3) {
            // Clean up parts
            for (auto& part : parts) {
                boost::trim(part);
            }
            
            json entry;
            entry["cleared"] = parts[0];
            entry["pending"] = parts[1];
            entry["lastCleared"] = parts.size() > 2 ? parts[2] : "";
            entry["account"] = parts.size() > 3 ? parts[3] : "";
            entry["level"] = level;
            
            result.push_back(entry);
        }
    }
    
    return result;
}

bool LedgerManager::addTransaction(const json& transactionData) {
    // Validate required fields
    if (!transactionData.contains("date") || !transactionData.contains("payee") || 
        !transactionData.contains("postings") || !transactionData["postings"].is_array() || 
        transactionData["postings"].empty()) {
        throw std::invalid_argument("Invalid transaction data: missing required fields");
    }
    
    // Create transaction object
    Transaction transaction;
    transaction.date = transactionData["date"].get<std::string>();
    transaction.payee = transactionData["payee"].get<std::string>();
    transaction.cleared = transactionData.value("isCleared", false);
    
    // Add postings
    for (const auto& postingData : transactionData["postings"]) {
        if (!postingData.contains("account")) {
            throw std::invalid_argument("Invalid posting data: missing account");
        }
        
        Posting posting;
        posting.account = postingData["account"].get<std::string>();
        posting.amount = postingData.value("amount", "");
        posting.comment = postingData.value("comment", "");
        
        transaction.postings.push_back(posting);
    }
    
    // Format the transaction
    std::string formattedTransaction = formatTransaction(transaction);
    
    // Append to the ledger file
    std::string content = parseLedgerFile();
    std::string newContent = content + "\n" + formattedTransaction;
    
    // Write back to the file
    writeLedgerFile(newContent);
    
    // Update reports
    updateReports();
    
    return true;
}

bool LedgerManager::updateTransaction(int index, const json& transactionData) {
    // Validate required fields
    if (!transactionData.contains("date") || !transactionData.contains("payee") || 
        !transactionData.contains("postings") || !transactionData["postings"].is_array() || 
        transactionData["postings"].empty()) {
        throw std::invalid_argument("Invalid transaction data: missing required fields");
    }
    
    // Get current content and parse transactions
    std::string content = parseLedgerFile();
    auto transactions = parseTransactions(content);
    
    if (index < 0 || index >= static_cast<int>(transactions.size())) {
        throw std::out_of_range("Transaction index out of range");
    }
    
    // Create updated transaction
    Transaction transaction;
    transaction.date = transactionData["date"].get<std::string>();
    transaction.payee = transactionData["payee"].get<std::string>();
    transaction.cleared = transactionData.value("isCleared", false);
    
    // Add postings
    for (const auto& postingData : transactionData["postings"]) {
        if (!postingData.contains("account")) {
            throw std::invalid_argument("Invalid posting data: missing account");
        }
        
        Posting posting;
        posting.account = postingData["account"].get<std::string>();
        posting.amount = postingData.value("amount", "");
        posting.comment = postingData.value("comment", "");
        
        transaction.postings.push_back(posting);
    }
    
    // Format the transaction
    std::string formattedTransaction = formatTransaction(transaction);
    
    // Find the start and end of the transaction to replace
    int startLine = transactions[index].startLine;
    int endLine = (index < static_cast<int>(transactions.size()) - 1) 
                   ? transactions[index + 1].startLine - 1 
                   : -1;
    
    // Split content into lines
    std::vector<std::string> lines;
    std::istringstream stream(content);
    std::string line;
    
    while (std::getline(stream, line)) {
        lines.push_back(line);
    }
    
    // Create new content
    std::string newContent;
    
    if (endLine == -1) {
        // This is the last transaction
        for (int i = 0; i < startLine; i++) {
            newContent += lines[i] + "\n";
        }
        
        newContent += formattedTransaction;
    } else {
        // This is not the last transaction
        for (int i = 0; i < startLine; i++) {
            newContent += lines[i] + "\n";
        }
        
        newContent += formattedTransaction;
        
        for (size_t i = endLine + 1; i < lines.size(); i++) {
            newContent += lines[i] + "\n";
        }
    }
    
    // Write back to the file
    writeLedgerFile(newContent);
    
    // Update reports
    updateReports();
    
    return true;
}

bool LedgerManager::deleteTransaction(int index) {
    // Get current content and parse transactions
    std::string content = parseLedgerFile();
    auto transactions = parseTransactions(content);
    
    if (index < 0 || index >= static_cast<int>(transactions.size())) {
        throw std::out_of_range("Transaction index out of range");
    }
    
    // Find the start and end of the transaction to delete
    int startLine = transactions[index].startLine;
    int endLine = (index < static_cast<int>(transactions.size()) - 1) 
                   ? transactions[index + 1].startLine - 1 
                   : -1;
    
    // Split content into lines
    std::vector<std::string> lines;
    std::istringstream stream(content);
    std::string line;
    
    while (std::getline(stream, line)) {
        lines.push_back(line);
    }
    
    // Create new content without the deleted transaction
    std::string newContent;
    
    if (endLine == -1) {
        // This is the last transaction
        for (int i = 0; i < startLine; i++) {
            newContent += lines[i] + "\n";
        }
    } else {
        // This is not the last transaction
        for (int i = 0; i < startLine; i++) {
            newContent += lines[i] + "\n";
        }
        
        for (size_t i = endLine + 1; i < lines.size(); i++) {
            newContent += lines[i] + "\n";
        }
    }
    
    // Write back to the file
    writeLedgerFile(newContent);
    
    // Update reports
    updateReports();
    
    return true;
}

void LedgerManager::updateReports() {
    const auto& config = Config::getInstance();
    std::string cmd = "bash " + config.getUpdateReportsScript();
    
    try {
        execCommand(cmd);
    } catch (const std::exception& e) {
        std::cerr << "Warning: Failed to update reports: " << e.what() << std::endl;
    }
}

} // namespace ledger_api 