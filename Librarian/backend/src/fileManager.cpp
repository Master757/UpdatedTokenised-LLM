#include "../include/fileManager.h"
#include <filesystem>
#include <iterator>

FileManager::FileManager() {
    std::filesystem::create_directory(path);
}

void FileManager::writeMessage(int userId, const std::string& msg) {
    std::ofstream out(path + std::to_string(userId) + ".txt", std::ios::app);
    if (out.is_open()) {
        out << msg << "\n";
        out.close();
    }
}

std::string FileManager::readHistory(int userId) {
    std::ifstream in(path + std::to_string(userId) + ".txt");
    if (!in.is_open()) return "";

    std::string content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    return content;
}

void FileManager::clearHistory(int userId) {
    std::filesystem::remove(path + std::to_string(userId) + ".txt");
}

void FileManager::addBook(const std::string& bookName) {
    sqlite3* db;
    if (sqlite3_open("../../library.db", &db) == SQLITE_OK) {
        std::string sql = "UPDATE Books SET status = 'Available' WHERE title = '" + bookName + "';";
        sqlite3_exec(db, sql.c_str(), 0, 0, 0);
        sqlite3_close(db);
    }
}

void FileManager::removeBook(const std::string& bookName) {
    sqlite3* db;/*this is a C library which we need to use to manage DB*/
    if (sqlite3_open("../../library.db", &db) == SQLITE_OK) {
        std::string sql = "UPDATE Books SET status = 'Checked Out' WHERE title = '" + bookName + "';";
        sqlite3_exec(db, sql.c_str(), 0, 0, 0);
        sqlite3_close(db);
    }
}

std::string FileManager::getBookList() {
    sqlite3* db;
    std::string catalog = "";
    if (sqlite3_open("../../library.db", &db) == SQLITE_OK) {
        std::string sql = "SELECT title, author, genre FROM Books WHERE status = 'Available';";
        
        auto callback = [](void* data, int argc, char** argv, char** azColName) -> int {
            std::string* cat = static_cast<std::string*>(data);
            if (argc >= 3) {
                *cat += "- " + std::string(argv[0]) + " by " + std::string(argv[1]) + " [" + std::string(argv[2]) + "]\n";
            }
            return 0;
        };

        sqlite3_exec(db, sql.c_str(), callback, &catalog, 0);
        sqlite3_close(db);
    }
    
    if (catalog.empty()) {
        catalog = "No books are currently available.";
    }
    return catalog;
}