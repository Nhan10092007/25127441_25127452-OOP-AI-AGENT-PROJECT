#pragma once
#include "tool.h"
#include "tool_policy.h"
#include <unordered_map>
#include <string>
#include <memory>
#include <functional>

class ToolRegistry {
private:
    std::unordered_map<std::string, std::function<std::unique_ptr<Tool>()>> factories;
    ToolPolicy policy;

public:
    template <typename T>
    void registerTool(const std::string& toolName) {
        factories[toolName] = []() -> std::unique_ptr<Tool> {
            return std::make_unique<T>(); 
        };
    }
    void registerToolFactory(const std::string& toolName, std::function<std::unique_ptr<Tool>()> factory);
    void setPolicy(const ToolPolicy& p);
    std::unique_ptr<Tool> createTool(const std::string& toolName) const;
    std::string getToolsDescription() const;
};