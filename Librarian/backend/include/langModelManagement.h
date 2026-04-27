#pragma once

#include <string>
#include <nlohmann/json.hpp>/*used for handling JSON data, i am using Linux so this is pre-installed*/
using json = nlohmann::json;

class FileManager;
/*we will use Gemini API key to retreve LLM contemplations*/
class LanguageModel{
    private:
        std::string API_KEY="";
        std::string queryPrompt="";
        std::string libraryRules="";
        std::string postRequests(const std::string &url, const std::string &payload);
        std::string extractAnswer(const std::string &response, int modelType);
    public:
        LanguageModel(const std::string &apiKey);
        std::string getContext(const std::string &rawHistory);
        std::string askLLM(const std::string &userQuery, const std::string &context, 
            const std::string &catalog, FileManager& fm);
        std::string processResponse(const std::string& response, FileManager& fm);
        
};