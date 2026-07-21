#pragma once
#include <string>
#include <variant>
#include "nlohmann/json.hpp"

using json = nlohmann::json;


struct ToolCallAction {
    std::string toolName;   
    std::string args;       
};

struct FinishAction {
    std::string finalAnswer; 
};

struct ErrorAction {
    std::string message;    
};

using Action = std::variant<ToolCallAction, FinishAction, ErrorAction>;
inline json actionToJson(const Action& action) {
    return std::visit([](const auto& a) -> json {
        using T = std::decay_t<decltype(a)>;
        if constexpr (std::is_same_v<T, ToolCallAction>) {
            return json{{"type", "tool_call"}, {"tool", a.toolName}, {"args", a.args}};
        } else if constexpr (std::is_same_v<T, FinishAction>) {
            return json{{"type", "finish"}, {"result", a.finalAnswer}};
        } else {
            return json{{"type", "error"}, {"message", a.message}};
        }
    }, action);
}