
#include <unistd.h>
#include "TCP_server.h"
#include <poll.h>
#include <cstring>
#include <cerrno>
#include <iostream>
#include <thread>
#include <chrono>
#include <arpa/inet.h>
#include "DatabaseManager.h"
TCPServer::TCPServer(int port) : port_(port), server_fd_(-1) {
    memset(&addr_, 0, sizeof(addr_));  // Resetting all the memoery values
}

TCPServer::~TCPServer() { //destructor to close the server FD when NSIG comes
    if (server_fd_ != -1) close(server_fd_);
}

bool TCPServer::init() {
    server_fd_ = socket(AF_INET6, SOCK_STREAM, 0);
    if (server_fd_ == -1) {
        std::cerr << "--> socket() failed: " << strerror(errno) << std::endl;
        return false;
    }

    int opt = 1;
    setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    addr_.sin6_family = AF_INET6;
    addr_.sin6_port = htons(port_);
    addr_.sin6_addr = in6addr_any;

    if (bind(server_fd_, (struct sockaddr*)&addr_, sizeof(addr_)) == -1) {
        std::cerr << "--> bind() failed on port " << port_ << ": " << strerror(errno) << std::endl;
        close(server_fd_);
        server_fd_ = -1;
        return false;
    }

    std::cout << "--> Bound to [::]:" << port_ << std::endl;
    return true;
}

bool TCPServer::startListening() {
    return (server_fd_ != -1 && listen(server_fd_, 5) != -1);
}

int TCPServer::acceptConnection() {
    struct pollfd pfd = {server_fd_, POLLIN, 0};
    if (poll(&pfd, 1, 5000) == 0) return -2;  // 5s timeout

    socklen_t len = sizeof(addr_);
    int fd = accept(server_fd_, (struct sockaddr*)&addr_, &len);
    if (fd > 0) {
        char ip[INET6_ADDRSTRLEN];
        inet_ntop(AF_INET6, &addr_.sin6_addr, ip, sizeof(ip));
        std::cout << "\nClient connected from [" << ip << "]:" << ntohs(addr_.sin6_port) << std::endl;
    }
    return fd;
}

std::string TCPServer::receiveMessage(int fd) {
    char buf[4096] = {0};
    struct timeval tv = {60, 0};
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    
    ssize_t n = recv(fd, buf, sizeof(buf)-1, 0);
    if (n <= 0) return "";
    
    buf[n] = '\0';
    std::string msg(buf);
    while (!msg.empty() && (msg.back() == '\n' || msg.back() == '\r')) msg.pop_back();
    return msg;
}

bool TCPServer::sendFullMessage(int fd, const std::string& msg) {
    return (send(fd, msg.c_str(), msg.size(), 0) == (ssize_t)msg.size());
}

void TCPServer::run(LLMManager& ai) {
    if (!init() || !startListening()) {
        std::cerr << "[FATAL] Server init failed" << std::endl;
        return;
    }

    DatabaseManager db;

    while (true) {
        int client = acceptConnection();
        if (client == -2) {
            std::cout << "# Waiting for Connections..." << std::endl;
            continue;
        }
        if (client < 0) continue;

        sendFullMessage(client, "Ask me anything (type 'quit' to exit):\n");

        while (true) {
            std::string raw = receiveMessage(client);
            if (raw.empty() || raw == "quit") break;

            // Parse session ID from format: "SESSION_ID|prompt"
            // Falls back to "default" for raw telnet connections
            std::string sessionId = "default";
            std::string input = raw;
            size_t pipe = raw.find('|');
            if (pipe != std::string::npos) {
                sessionId = raw.substr(0, pipe);
                input = raw.substr(pipe + 1);
            }

            std::cout << "[Session " << sessionId << "] " << input << "\n";

            if (db.getBalance(sessionId) <= 0) {
                sendFullMessage(client, "ran out of tokens to generate\n♦");
                continue;
            }

            sendFullMessage(client, "--> Thinking...\n");
            
            std::string response = ai.queryModel(input); 
            std::cout << response + "\n";
            
            // Database and Token Logic — per session
            int tokens_used = db.countTokens(input) + db.countTokens(response);
            db.deductBalance(sessionId, tokens_used * 0.1); // 0.1 cents per word token
            db.logChat(sessionId, input, response);
            
            std::string finalResponse = "AI: " + response + "\n";
            if (!sendFullMessage(client, finalResponse)) {
                break;
            }else{
                sendFullMessage(client, "♦");
            }
        }
        close(client);
        std::cout << "disconnected\n" << std::endl;
    }
}