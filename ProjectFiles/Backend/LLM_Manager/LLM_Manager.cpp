#include "LLM_Manager.h"
#include <curl/curl.h>
#include <iostream>
#include <sstream>
#include <iomanip>

LLMManager::LLMManager(std::string modelName) : _modelName(modelName) {}

size_t LLMManager::WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

std::string escapeJson(const std::string& input) {
    std::ostringstream oss;
    for (char c : input) {
        switch (c) {
            case '"':  oss << "\\\""; break;
            case '\\': oss << "\\\\"; break;
            case '\n': oss << "\\n";  break;
            case '\r': oss << "\\r";  break;
            default: oss << c;
        }
    }
    return oss.str();
}

std::string LLMManager::queryModel(const std::string& prompt) {
    CURL* curl = curl_easy_init();
    std::string responseBuffer;

    if (!curl) return "XXXX curl init failed";
    const char* env_url = std::getenv("OLLAMA_HOST");
    std::string baseUrl = (env_url != nullptr) ? std::string(env_url) : "http://127.0.0.1:11434";
    std::string fullUrl = baseUrl + "/api/generate";
    curl_easy_setopt(curl, CURLOPT_URL, fullUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 300L);

    std::string json_data = "{\"model\":\"" + _modelName + 
                            "\",\"prompt\":\"" + escapeJson(prompt) + 
                            "\",\"stream\":false}";

    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_data.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseBuffer);

    std::cout << "_________Thinking..." << std::endl;
    CURLcode res = curl_easy_perform(curl);
    
    if (res != CURLE_OK) {
        responseBuffer = "ERROR: Ollama unreachable";
    } else {
        size_t start = responseBuffer.find("\"response\":\"");
        if (start != std::string::npos) {
            start += 12; // move past "response":"
            
            // --- THE FIX STARTS HERE ---
            size_t end = start;
            while ((end = responseBuffer.find("\"", end)) != std::string::npos) {
                if (end > 0 && responseBuffer[end - 1] == '\\') {
                    end++; // Move past this escaped quote and keep searching
                } else {
                    break; // We found the true, unescaped closing quote
                }
            }

            if (end != std::string::npos) {
                responseBuffer = responseBuffer.substr(start, end - start);
                
                size_t pos = 0;
                while ((pos = responseBuffer.find("\\n", pos)) != std::string::npos) {
                    responseBuffer.replace(pos, 2, "\n");
                    pos += 1;
                }
                
                pos = 0; // Reset pos for the next replacement loop
                while ((pos = responseBuffer.find("\\\"", pos)) != std::string::npos) {
                    responseBuffer.replace(pos, 2, "\"");
                    pos += 1;
                }
            }
        }
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    return responseBuffer;
}