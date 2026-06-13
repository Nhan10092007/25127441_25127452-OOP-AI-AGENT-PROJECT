#pragma once

#include"llm_client.h"

struct LLMConfig{
    std::string base_URL;
    std::string model_name;
    float temperature;
    int num_predict;
};

// For Ollanma API:
class OllamaClient : public LLMClient{
private:
    std::string _baseURL;
    std::string _modelName;
    float _temperature;
    int _numPredict;
public:
    OllamaClient(const LLMConfig& config);
    ~OllamaClient();
    LLMResponse sendRequest(const std::vector<Message> &messages) override;
};


// For other API like OpenAIAPI,...:
