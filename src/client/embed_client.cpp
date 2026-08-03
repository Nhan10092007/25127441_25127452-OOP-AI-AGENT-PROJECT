#include"embed_client.h"
#include"curl/curl.h"
#include"nlohmann/json.hpp"

using json = nlohmann::json;

EmbeddingClient::EmbeddingClient(const EmbeddingConfig& config): _modelName(config.model_name), _baseURL(config.base_URL){}

EmbeddingClient::~EmbeddingClient() = default;

std::vector<double> EmbeddingClient::embed(const std::string& text){
    
}