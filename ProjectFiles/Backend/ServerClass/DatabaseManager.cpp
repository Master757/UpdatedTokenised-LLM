#include "DatabaseManager.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <sys/stat.h>

DatabaseManager::DatabaseManager() {
    dataDir = "sessions";
    defaultBalance = 100.0;

    // Create sessions directory if it doesn't exist
    mkdir(dataDir.c_str(), 0755);
}

std::string DatabaseManager::getBalanceFile(const std::string& sessionId) {
    return dataDir + "/balance_" + sessionId + ".txt";
}

std::string DatabaseManager::getHistoryFile(const std::string& sessionId) {
    return dataDir + "/history_" + sessionId + ".txt";
}

void DatabaseManager::loadBalance(const std::string& sessionId) {
    // Already loaded in memory
    if (balances.find(sessionId) != balances.end()) return;

    std::string file = getBalanceFile(sessionId);
    std::ifstream infile(file);
    if (infile.is_open()) {
        double bal;
        infile >> bal;
        balances[sessionId] = bal;
        infile.close();
    } else {
        // New session — give them the default balance
        balances[sessionId] = defaultBalance;
        std::ofstream outfile(file);
        outfile << defaultBalance;
        outfile.close();
    }
}

double DatabaseManager::getBalance(const std::string& sessionId) {
    loadBalance(sessionId);
    return balances[sessionId];
}

void DatabaseManager::deductBalance(const std::string& sessionId, double amount) {
    loadBalance(sessionId);
    balances[sessionId] -= amount;
    std::ofstream outfile(getBalanceFile(sessionId));
    outfile << balances[sessionId];
    outfile.close();
    std::cout << "[Session " << sessionId << "] Balance: " << balances[sessionId] << std::endl;
}

void DatabaseManager::logChat(const std::string& sessionId, const std::string& userMsg, const std::string& botMsg) {
    std::ofstream outfile(getHistoryFile(sessionId), std::ios::app);
    if (outfile.is_open()) {
        outfile << "User: " << userMsg << "\n";
        outfile << "Bot: " << botMsg << "\n\n";
        outfile.close();
    }
}

//using a word vector to count tokens
int DatabaseManager::countTokens(const std::string& text) {
    std::istringstream stream(text);
    std::string word;
    int count = 0;
    while (stream >> word) {
        count++;
    }
    return count;
}
