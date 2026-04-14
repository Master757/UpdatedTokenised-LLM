#ifndef LLM_MANAGER_H
#define LLM_MANAGER_H

#include <string>

class LLMManager {
public:
    // Constructor allows you to change models easily (defaulting to Mistral)
    LLMManager(std::string modelName = "mistral");
    
    // The main function the Server will call
    // It takes the raw prompt and returns the AI's response text
    std::string queryModel(const std::string& prompt);

private:
    std::string _modelName;

    // libcurl needs a static callback function to handle the incoming data stream
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);
};

#endif