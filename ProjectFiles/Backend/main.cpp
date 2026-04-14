// main.cpp
#include <iostream>
#include "ServerClass/TCP_server.h"  
#include "LLM_Manager/LLM_Manager.h" 

int main() {
    std::string modelName = "potato_companion"; // Change it once here
    LLMManager aiBrain(modelName);

    TCPServer server(8080);

    std::cout << "\n=== AI TCP SERVER LAUNCHING ===" << std::endl;
    std::cout << "Model: " << modelName << std::endl; // Now it tells the truth!
    std::cout << "Port: 8080" << std::endl;
    std::cout << "================================\n" << std::endl;

    server.run(aiBrain); 
    return 0;
}