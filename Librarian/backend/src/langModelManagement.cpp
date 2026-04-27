#include "../include/langModelManagement.h"
#include "../include/fileManager.h"
#include <iostream>
#include <curl/curl.h>


using LM_ = LanguageModel;
/*libcurl callbacks t write it inside of a string itself [scourced]*/
size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

LM_::LanguageModel(const std::string &apiKey) : API_KEY(apiKey) {
    this->libraryRules = 
        "You are a helpful Library Assistant. "
        "Always be professional and respectful. "
        "Always check the catalog before telling the user if the book is available or not. Don't guess. "
        "1. If a user wants to checkout/get a book that is ALREADY in the catalog: It is FREE, and you MUST output [CALL: removeBook(bookName)]. "
        "2. If a user wants to request/issue a NEW book NOT in the catalog: Charge a $2 fee, and you MUST output [CALL: addBook(bookName)]. "
        "3. If a user returns a book: It is FREE, and you MUST output [CALL: addBook(bookName)]. "
        "Suggest books here and there based on the catalog in your conversations, keep in mind the user's taste in books and genres, try your best to give them book suggestions that they might like based on their previous interactions.";

    std::cout << "--> Language model initialized using Gemini API Key" << std::endl;
}

std::string LM_::postRequests(const std::string &url, const std::string &payload) {
    CURL* curl = curl_easy_init();
    std::string readBuffer;

    if (curl) {
        struct curl_slist* headers = NULL; /*dont know any headers as such for this specific API call */
        headers = curl_slist_append(headers, "Content-Type: application/json");

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);


        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "CURL Error: " << curl_easy_strerror(res) << std::endl;
        }

        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }
    return readBuffer;
}

std::string LM_::extractAnswer(const std::string &response, int modelType) {
    try {
        auto j = json::parse(response);
        
        if (modelType == 0) {
            // Navigation: candidates -> content -> parts -> text (Gemini), this is th sturutre of the Gemini reposne JSON payload
            if (j.contains("candidates") && !j["candidates"].empty()) {
                return j["candidates"][0]["content"]["parts"][0]["text"];
            }
            
            // Error handling if Gemini blocks the prompt or hits a safety filter whcih is very common
            if (j.contains("error")) {
                return "API Error: " + j["error"]["message"].get<std::string>();
            }
        } else if (modelType == 1) {
            // Navigation for Ollama API (SLM)
            if (j.contains("response")) {
                return j["response"].get<std::string>();
            }
        }

    } catch (json::parse_error& e) {
        std::cerr << "JSON Parse Error: " << e.what() << std::endl;
    }
    return "I'm sorry, I couldn't process that request.";
}

std::string LM_::askLLM(const std::string &userQuery, const std::string &context, 
    const std::string &catalog, FileManager& fm){

        std::string finalPrompt = "Context summary: "+context+
                                "\nLibrary Catalog: "+catalog+
                                "\nUser Query: "+userQuery;
        
        json payload = {
            {"contents", {{
                {"parts", {{
                    {"text", finalPrompt}
                }}}
            }}},
            {"system_instruction", {
                {"parts", {
                    {"text", this->libraryRules}
                }}
            }}
        };

        std::string url = "https://generativelanguage.googleapis.com/v1beta/models/gemini-1.5-flash:generateContent?key=" + this->API_KEY;

        std::string response = postRequests(url, payload.dump());
        std::string extracted = extractAnswer(response, 0); // 0 means LLM (Gemini or any other LLM)
        return processResponse(extracted, fm);
}

std::string LM_::getContext(const std::string &rawHistory) {
    if (rawHistory.empty()) return "No previous conversation."; //we wil not hit due to introductory message [not yet mandatory]
    
    std::string prompt = "Summarize the following conversation history for context: " + rawHistory +
        "Keep the details such as names and book names clearly and well structured. The rules and needs are very important dont miss them out.";
    
    json payload = {
        {"model", "gemma"}, // or phi4 [i will use gemma for fast and efficient use here]
        {"prompt", prompt},
        {"stream", false} // We want the full answer at once
    };

    std::string url = "http://localhost:11434/api/generate";
    std::string response = postRequests(url, payload.dump());
    
    return extractAnswer(response, 1); // 1 means SLM (Ollama in this case)
}

std::string LM_::processResponse(const std::string& response, FileManager& fm) {
    std::string cleanResponse = response;
    
    // Check for removeBook to remove the books from the DB
    size_t removePos = cleanResponse.find("[CALL: removeBook(");
    if (removePos != std::string::npos) {
        size_t start = removePos + 18; // length of "[CALL: removeBook( which gemini gives after the name of the book is given"
        size_t end = cleanResponse.find(")]", start);
        if (end != std::string::npos) {
            std::string bookName = cleanResponse.substr(start, end - start);
            fm.removeBook(bookName);
            // Optionally remove the tag from the user's view
            cleanResponse.erase(removePos, end - removePos + 2);
        }
    }

    // Check for addBook
    size_t addPos = cleanResponse.find("[CALL: addBook(");
    if (addPos != std::string::npos) {
        size_t start = addPos + 15; // length of "[CALL: addBook("
        size_t end = cleanResponse.find(")]", start);
        if (end != std::string::npos) {
            std::string bookName = cleanResponse.substr(start, end - start);
            fm.addBook(bookName);
            cleanResponse.erase(addPos, end - addPos + 2);
        }
    }

    return cleanResponse;
}
