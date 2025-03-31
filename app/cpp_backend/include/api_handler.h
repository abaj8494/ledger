#ifndef LEDGER_API_API_HANDLER_H
#define LEDGER_API_API_HANDLER_H

#include <string>
#include <crow.h>
#include "ledger_manager.h"

namespace ledger_api {

class ApiHandler {
public:
    ApiHandler() = default;
    ~ApiHandler() = default;

    // Initialize routes for the Crow app
    void initRoutes(crow::SimpleApp& app);

private:
    // Handler function to setup CORS
    void setCorsHeaders(crow::response& res);

    // API route handlers
    crow::response getSummary();
    crow::response getTransactions(const crow::request& req);
    crow::response getAccounts();
    crow::response getBalance();
    crow::response getRegister();
    crow::response getBudget();
    crow::response getCleared();
    crow::response getTransaction(int index);
    crow::response addTransaction(const crow::request& req);
    crow::response updateTransaction(int index, const crow::request& req);
    crow::response deleteTransaction(int index);
};

} // namespace ledger_api

#endif // LEDGER_API_API_HANDLER_H 