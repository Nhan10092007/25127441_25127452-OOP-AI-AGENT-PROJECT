#pragma once
#include<string>
#include<vector>

// Do dữ liệu đầu vào có thể là multimodal nên sẽ có thể có img và nó sẽ được lưu trữ dưới dạng Base64
struct Message{
    std::string role;
    std::string content;
    std::vector<std::string> images;
};

struct LLMResponse{
    std::string response;
    int prompt_token;
    int answer_token;
    int total_token;
    bool success;
};

// Abstract interface
class LLMClient{
public:
    virtual LLMResponse sendRequest(const std::vector<Message> &messages) = 0;
    virtual ~LLMClient() = default;
};