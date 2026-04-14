#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <string>
#include <map>

class DatabaseManager {
private:
    std::map<std::string, double> balances;   // per-session balances
    std::string dataDir;                       // directory for balance/history files
    double defaultBalance;

    std::string getBalanceFile(const std::string& sessionId);
    std::string getHistoryFile(const std::string& sessionId);
    void loadBalance(const std::string& sessionId);

public:
    DatabaseManager();
    double getBalance(const std::string& sessionId);
    void deductBalance(const std::string& sessionId, double amount);
    void logChat(const std::string& sessionId, const std::string& userMsg, const std::string& botMsg);
    int countTokens(const std::string& text);
};

#endif
