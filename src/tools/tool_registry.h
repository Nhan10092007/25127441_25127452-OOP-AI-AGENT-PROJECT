#pragma once
#include "tool.h"
#include <unordered_map>
#include <string>
#include <memory>
#include <functional>

class ToolRegistry {
private:
    std::unordered_map<std::string, std::function<std::unique_ptr<Tool>()>> factories;

public:
   
    template <typename T>
    void registerTool(const std::string& toolName) {
        factories[toolName] = []() -> std::unique_ptr<Tool> {
            return std::make_unique<T>(); 
        };
    }
    std::unique_ptr<Tool> createTool(const std::string& toolName) {
        auto it = factories.find(toolName);
        if (it != factories.end()) {
            return it->second(); 
        }
        return nullptr; 
    }
};