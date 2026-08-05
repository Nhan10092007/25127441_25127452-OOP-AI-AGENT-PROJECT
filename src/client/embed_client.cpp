#include"embed_client.h"
#include"curl/curl.h"
#include"nlohmann/json.hpp"
#include<stdexcept>

using json = nlohmann::json;

EmbeddingClient::EmbeddingClient(const EmbeddingConfig& config): _modelName(config.model_name), _baseURL(config.base_URL){}

EmbeddingClient::~EmbeddingClient() = default;

static size_t embedCallBack(void* contents, size_t size, size_t nbemb, void* userptr){
    size_t sumbyte = size * nbemb;
    std::string* container = static_cast<std::string*>(userptr);
    char* trueData = static_cast<char*>(contents);
    container->append(trueData, sumbyte);
    return sumbyte;
}

std::vector<float> EmbeddingClient::embed(const std::string& text){
    if(text.empty()){
        throw std::runtime_error("Empty text to embedding!");
    }
    json json_payload;
    json_payload["text"] = text;
 
    CURL* curl = curl_easy_init();
    curl_slist* header = nullptr;
    header = curl_slist_append(header, "ngrok-skip-browser-warning: true");
    header = curl_slist_append(header, "Content-Type: application/json");
    header = curl_slist_append(header, "Accept: application/json");
 
    curl_easy_setopt(curl, CURLOPT_URL, _baseURL.c_str());
    curl_easy_setopt(curl, CURLOPT_POST, 1);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);
 
    std::string payload_string = json_payload.dump();
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload_string.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, embedCallBack);
 
    std::string responseData;
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseData);
 
    CURLcode result = curl_easy_perform(curl);
    curl_slist_free_all(header);
    curl_easy_cleanup(curl);
 
    if(result != CURLE_OK){
        throw std::runtime_error("Libcurl Error: " + std::string(curl_easy_strerror(result)));
    }
 
    try{
        json data = json::parse(responseData);
 
        if(data.contains("error")){
            throw std::runtime_error("Embedding API Error: " + data["error"].get<std::string>());
        }
        if(!data.value("success", false)){
            throw std::runtime_error("Embedding API return success=false");
        }
 
        std::vector<float> embedding = data["embedding"].get<std::vector<float>>();
        if(embedding.empty()){
            throw std::runtime_error("Embedding API return empty result");
        }
        return embedding;
    }
    catch(const json::parse_error& e){
        throw std::runtime_error("Malformed JSON Error: " + std::string(e.what()) + "\nRaw Data: " + responseData);
    }
    catch(const std::exception& e){
        throw std::runtime_error("JSON Error: " + std::string(e.what()));
    }
}