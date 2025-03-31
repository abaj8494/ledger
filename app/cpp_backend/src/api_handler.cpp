#include "api_handler.h"
#include <iostream>

namespace ledger_api {

void ApiHandler::initRoutes(crow::SimpleApp& app) {
    // Register API routes
    CROW_ROUTE(app, "/api/summary").methods(crow::HTTPMethod::GET)
    ([this](const crow::request& req) {
        try {
            return getSummary();
        } catch (const std::exception& e) {
            crow::response res(500);
            res.write("{\"error\":\"" + std::string(e.what()) + "\"}");
            return res;
        }
    });

    CROW_ROUTE(app, "/api/transactions").methods(crow::HTTPMethod::GET)
    ([this](const crow::request& req) {
        try {
            return getTransactions(req);
        } catch (const std::exception& e) {
            crow::response res(500);
            res.write("{\"error\":\"" + std::string(e.what()) + "\"}");
            return res;
        }
    });

    CROW_ROUTE(app, "/api/accounts").methods(crow::HTTPMethod::GET)
    ([this](const crow::request& req) {
        try {
            return getAccounts();
        } catch (const std::exception& e) {
            crow::response res(500);
            res.write("{\"error\":\"" + std::string(e.what()) + "\"}");
            return res;
        }
    });

    CROW_ROUTE(app, "/api/balance").methods(crow::HTTPMethod::GET)
    ([this](const crow::request& req) {
        try {
            return getBalance();
        } catch (const std::exception& e) {
            crow::response res(500);
            res.write("{\"error\":\"" + std::string(e.what()) + "\"}");
            return res;
        }
    });

    CROW_ROUTE(app, "/api/register").methods(crow::HTTPMethod::GET)
    ([this](const crow::request& req) {
        try {
            return getRegister();
        } catch (const std::exception& e) {
            crow::response res(500);
            res.write("{\"error\":\"" + std::string(e.what()) + "\"}");
            return res;
        }
    });

    CROW_ROUTE(app, "/api/budget").methods(crow::HTTPMethod::GET)
    ([this](const crow::request& req) {
        try {
            return getBudget();
        } catch (const std::exception& e) {
            crow::response res(500);
            res.write("{\"error\":\"" + std::string(e.what()) + "\"}");
            return res;
        }
    });

    CROW_ROUTE(app, "/api/cleared").methods(crow::HTTPMethod::GET)
    ([this](const crow::request& req) {
        try {
            return getCleared();
        } catch (const std::exception& e) {
            crow::response res(500);
            res.write("{\"error\":\"" + std::string(e.what()) + "\"}");
            return res;
        }
    });

    CROW_ROUTE(app, "/api/transactions/<int>").methods(crow::HTTPMethod::GET)
    ([this](const crow::request& req, int index) {
        try {
            return getTransaction(index);
        } catch (const std::exception& e) {
            if (dynamic_cast<const std::out_of_range*>(&e)) {
                crow::response res(404);
                res.write("{\"error\":\"Transaction not found\"}");
                return res;
            }
            crow::response res(500);
            res.write("{\"error\":\"" + std::string(e.what()) + "\"}");
            return res;
        }
    });

    CROW_ROUTE(app, "/api/transactions").methods(crow::HTTPMethod::POST)
    ([this](const crow::request& req) {
        try {
            return addTransaction(req);
        } catch (const std::exception& e) {
            if (dynamic_cast<const std::invalid_argument*>(&e)) {
                crow::response res(400);
                res.write("{\"error\":\"" + std::string(e.what()) + "\"}");
                return res;
            }
            crow::response res(500);
            res.write("{\"error\":\"" + std::string(e.what()) + "\"}");
            return res;
        }
    });

    CROW_ROUTE(app, "/api/transactions/<int>").methods(crow::HTTPMethod::PUT)
    ([this](const crow::request& req, int index) {
        try {
            return updateTransaction(index, req);
        } catch (const std::exception& e) {
            if (dynamic_cast<const std::out_of_range*>(&e)) {
                crow::response res(404);
                res.write("{\"error\":\"Transaction not found\"}");
                return res;
            }
            if (dynamic_cast<const std::invalid_argument*>(&e)) {
                crow::response res(400);
                res.write("{\"error\":\"" + std::string(e.what()) + "\"}");
                return res;
            }
            crow::response res(500);
            res.write("{\"error\":\"" + std::string(e.what()) + "\"}");
            return res;
        }
    });

    CROW_ROUTE(app, "/api/transactions/<int>").methods(crow::HTTPMethod::DELETE)
    ([this](const crow::request& req, int index) {
        try {
            return deleteTransaction(index);
        } catch (const std::exception& e) {
            if (dynamic_cast<const std::out_of_range*>(&e)) {
                crow::response res(404);
                res.write("{\"error\":\"Transaction not found\"}");
                return res;
            }
            crow::response res(500);
            res.write("{\"error\":\"" + std::string(e.what()) + "\"}");
            return res;
        }
    });

    // Add OPTIONS route for CORS preflight requests
    CROW_ROUTE(app, "/api/<path>").methods(crow::HTTPMethod::OPTIONS)
    ([this](const crow::request&, const std::string&) {
        crow::response res(200);
        setCorsHeaders(res);
        return res;
    });
}

void ApiHandler::setCorsHeaders(crow::response& res) {
    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "X-Requested-With, Content-Type, Accept, Authorization");
}

crow::response ApiHandler::getSummary() {
    auto& ledgerManager = LedgerManager::getInstance();
    auto summaryData = ledgerManager.getAccountSummary();
    
    crow::response res;
    res.write(summaryData.dump());
    res.set_header("Content-Type", "application/json");
    setCorsHeaders(res);
    
    return res;
}

crow::response ApiHandler::getTransactions(const crow::request& req) {
    auto& ledgerManager = LedgerManager::getInstance();
    
    // Check for limit parameter
    int limit = 0;
    auto params = req.url_params;
    if (params.get("limit") != nullptr) {
        try {
            limit = std::stoi(params.get("limit"));
        } catch (...) {
            limit = 0;
        }
    }
    
    auto transactionsData = ledgerManager.getAllTransactions(limit);
    
    crow::response res;
    res.write(transactionsData.dump());
    res.set_header("Content-Type", "application/json");
    setCorsHeaders(res);
    
    return res;
}

crow::response ApiHandler::getAccounts() {
    auto& ledgerManager = LedgerManager::getInstance();
    auto accountsData = ledgerManager.getAllAccounts();
    
    crow::response res;
    res.write(accountsData.dump());
    res.set_header("Content-Type", "application/json");
    setCorsHeaders(res);
    
    return res;
}

crow::response ApiHandler::getBalance() {
    auto& ledgerManager = LedgerManager::getInstance();
    auto balanceData = ledgerManager.getBalanceReport();
    
    crow::response res;
    res.write(balanceData.dump());
    res.set_header("Content-Type", "application/json");
    setCorsHeaders(res);
    
    return res;
}

crow::response ApiHandler::getRegister() {
    auto& ledgerManager = LedgerManager::getInstance();
    auto registerData = ledgerManager.getRegisterReport();
    
    crow::response res;
    res.write(registerData.dump());
    res.set_header("Content-Type", "application/json");
    setCorsHeaders(res);
    
    return res;
}

crow::response ApiHandler::getBudget() {
    auto& ledgerManager = LedgerManager::getInstance();
    auto budgetData = ledgerManager.getBudgetReport();
    
    crow::response res;
    res.write(budgetData.dump());
    res.set_header("Content-Type", "application/json");
    setCorsHeaders(res);
    
    return res;
}

crow::response ApiHandler::getCleared() {
    auto& ledgerManager = LedgerManager::getInstance();
    auto clearedData = ledgerManager.getClearedReport();
    
    crow::response res;
    res.write(clearedData.dump());
    res.set_header("Content-Type", "application/json");
    setCorsHeaders(res);
    
    return res;
}

crow::response ApiHandler::getTransaction(int index) {
    auto& ledgerManager = LedgerManager::getInstance();
    std::string content = ledgerManager.parseLedgerFile();
    auto transaction = ledgerManager.getTransaction(content, index);
    
    // Convert transaction to JSON
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
    
    crow::response res;
    res.write(jsonTransaction.dump());
    res.set_header("Content-Type", "application/json");
    setCorsHeaders(res);
    
    return res;
}

crow::response ApiHandler::addTransaction(const crow::request& req) {
    auto& ledgerManager = LedgerManager::getInstance();
    
    try {
        auto jsonData = json::parse(req.body);
        
        if (ledgerManager.addTransaction(jsonData)) {
            crow::response res(201);
            res.write("{\"message\":\"Transaction added successfully\"}");
            res.set_header("Content-Type", "application/json");
            setCorsHeaders(res);
            return res;
        } else {
            crow::response res(500);
            res.write("{\"error\":\"Failed to add transaction\"}");
            res.set_header("Content-Type", "application/json");
            setCorsHeaders(res);
            return res;
        }
    } catch (const json::exception& e) {
        crow::response res(400);
        res.write("{\"error\":\"Invalid JSON: " + std::string(e.what()) + "\"}");
        res.set_header("Content-Type", "application/json");
        setCorsHeaders(res);
        return res;
    }
}

crow::response ApiHandler::updateTransaction(int index, const crow::request& req) {
    auto& ledgerManager = LedgerManager::getInstance();
    
    try {
        auto jsonData = json::parse(req.body);
        
        if (ledgerManager.updateTransaction(index, jsonData)) {
            crow::response res(200);
            res.write("{\"message\":\"Transaction updated successfully\"}");
            res.set_header("Content-Type", "application/json");
            setCorsHeaders(res);
            return res;
        } else {
            crow::response res(500);
            res.write("{\"error\":\"Failed to update transaction\"}");
            res.set_header("Content-Type", "application/json");
            setCorsHeaders(res);
            return res;
        }
    } catch (const json::exception& e) {
        crow::response res(400);
        res.write("{\"error\":\"Invalid JSON: " + std::string(e.what()) + "\"}");
        res.set_header("Content-Type", "application/json");
        setCorsHeaders(res);
        return res;
    }
}

crow::response ApiHandler::deleteTransaction(int index) {
    auto& ledgerManager = LedgerManager::getInstance();
    
    if (ledgerManager.deleteTransaction(index)) {
        crow::response res(200);
        res.write("{\"message\":\"Transaction deleted successfully\"}");
        res.set_header("Content-Type", "application/json");
        setCorsHeaders(res);
        return res;
    } else {
        crow::response res(500);
        res.write("{\"error\":\"Failed to delete transaction\"}");
        res.set_header("Content-Type", "application/json");
        setCorsHeaders(res);
        return res;
    }
}

} // namespace ledger_api 