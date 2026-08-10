#pragma once
#include<string>
#include<vector>

struct EmbeddingConfig{
    std::string model_name;
    std::string base_URL;
    double similarity_threshold;
};

class EmbeddingClient{
private:    
    std::string _modelName;
    std::string _baseURL;
public:
    EmbeddingClient(const EmbeddingConfig& config);
    ~EmbeddingClient();
    std::vector<float> embed(const std::string& text);
};