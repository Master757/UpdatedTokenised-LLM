#pragma once

#include <fstream>
#include <iostream>
#include <string>
#include <filesystem>
#include <sqlite3.h>

class FileManager {
    private:
        std::string path = "../../Archives/";

    public:
        FileManager();
        void writeMessage(int userId, const std::string& msg);
        std::string readHistory(int userId);
        void clearHistory(int userId);
        
        // Database operations
        void addBook(const std::string& bookName);
        void removeBook(const std::string& bookName);
        std::string getBookList();
};