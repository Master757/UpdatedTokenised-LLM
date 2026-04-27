#include <iostream>
#include <vector>
#include <poll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <string>
#include "../include/langModelManagement.h"
#include "../include/fileManager.h"

#define PORT 8080 /*you can chanage this*/
#define MAX_CLIENTS SOMAXCONN //to get maximum system capacity
#define API_KEY "YOUR_GEMINI_API_KEY"

int main() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    
    // Make it non-blocking
    fcntl(server_fd, F_SETFL, O_NONBLOCK);

    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    LanguageModel LLM(API_KEY);
    FileManager FM;//which we will also need
    
    bind(server_fd, (struct sockaddr*)&address, sizeof(address));
    listen(server_fd, MAX_CLIENTS);

    std::vector<pollfd> fds;
    fds.push_back({server_fd, POLLIN, 0});//first file descriptor[serverlistener]

    std::cout << "-->Library Server started on port " << PORT << "..." << std::endl;

    while (true) {
        int ret = poll(fds.data(), fds.size(), -1);
        if (ret < 0) break;

        for (size_t i = 0; i < fds.size(); i++) {
            if (fds[i].revents & POLLIN) {

                if (fds[i].fd == server_fd) {
                    // Accept new member connection
                    int new_socket = accept(server_fd, nullptr, nullptr);
                    fds.push_back({new_socket, POLLIN, 0});
                    std::cout << "New member joined the chat!" << std::endl;
                    std::string greeting = "AI: Hello! I am your Library Assistant. How can i help you today";
                    send(new_socket, greeting.c_str(), greeting.length(), 0);
                    FM.writeMessage(new_socket, greeting);
                } else {

                    // Handle message from a member
                    char buffer[1024] = {0};
                    int msg_len = read(fds[i].fd, buffer, 1024);
                    if (msg_len == 0) {
                        std::cout << "Member left the chat." << std::endl;
                        FM.clearHistory(fds[i].fd); // Cleanup history file
                        close(fds[i].fd);
                        fds.erase(fds.begin() + i);
                        i--; // Adjust index after erase
                    } else if (msg_len < 0) {
                        if (errno != EAGAIN && errno != EWOULDBLOCK) {
                            std::cerr << "Read error, closing connection." << std::endl;
                            close(fds[i].fd);
                            fds.erase(fds.begin() + i);
                            i--;
                        }
                    } else {

                        std::string msg(buffer, msg_len);
                        std::cout << "User asked: " << msg << std::endl;
                        
                        // Save user message to history
                        FM.writeMessage(fds[i].fd, "User: " + msg);

                        std::string history = FM.readHistory(fds[i].fd); 
                        std::string context = LLM.getContext(history);
                        std::string catalog_str = FM.getBookList();
                        std::string ansLLM_ = LLM.askLLM(msg, context, catalog_str, FM);
                        
                        // Save AI response to history
                        FM.writeMessage(fds[i].fd, "AI: " + ansLLM_);

                        std::string response = "AI: " + ansLLM_;
                        send(fds[i].fd, response.c_str(), response.length(), 0);
                    }
                }
            }
        }
    }
    return 0;
}