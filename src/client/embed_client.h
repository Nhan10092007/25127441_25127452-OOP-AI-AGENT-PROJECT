#pragma once
#include<string>
#include<vector>

struct EmbeddingConfig{
    std::string model_name;
    std::string base_URL;
};

class EmbeddingClient{
private:    
    std::string _modelName;
    std::string _baseURL;
public:
    EmbeddingClient(const EmbeddingConfig& config);
    ~EmbeddingClient();
    std::vector<double> embed(const std::string& text);
};