// ServerClass/TCP_server.h
#ifndef TCPSERVER_HPP  // ← MUST be #ifndef, not #define alone
#define TCPSERVER_HPP

#include <netinet/in.h>
#include <string>
#include "../LLM_Manager/LLM_Manager.h"

class TCPServer {
private:
    int port_;
    int server_fd_;
    struct sockaddr_in6 addr_;

public:
    TCPServer(int port);
    ~TCPServer();
    bool init();
    bool startListening();  // ← Fixed typo "Lisnting"
    int acceptConnection();
    bool sendFullMessage(int fd, const std::string& msg);
    std::string receiveMessage(int fd);
    void run(LLMManager& ai);
};

#endif  // ← MUST close guard