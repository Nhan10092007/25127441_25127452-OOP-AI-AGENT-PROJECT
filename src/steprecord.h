#pragma once

#include<string>
#include"nlohmann/json.hpp"
#include<optional>

using json = nlohmann::json;

struct StepRecord{
    int step_id;
    std::string thought;
    json action;
    std::optional<std::string> tool_result;
    std::optional<bool> success;
    std::optional<bool> loop_warning;
    std::optional<std::string> parse_error;
    int tokens_used;
    int answer_token;
    int prompt_token;
    int latency_ms;
};