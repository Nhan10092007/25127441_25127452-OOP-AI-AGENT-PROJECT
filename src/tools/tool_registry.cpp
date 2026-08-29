#include "tool_registry.h"

void ToolRegistry::setPolicy(const ToolPolicy& p) {
    policy = p;
}

std::unique_ptr<Tool> ToolRegistry::createTool(const std::string& toolName) const {
    if (!policy.isAllowed(toolName)) {
        return nullptr;  // Tool is denied by policy
    }
    auto it = factories.find(toolName);
    if (it != factories.end()) {
        return it->second(); 
    }
    return nullptr; 
}

std::string ToolRegistry::getToolsDescription() const {
    std::string result = "## AVAILABLE TOOLS:\n";
    for (const auto& [toolName, factory] : factories) {
        if (!policy.isAllowed(toolName)) {
            continue;  // Skip tools denied by policy
        }
        try {
            std::unique_ptr<Tool> tool = factory();
            result += "- " + tool->getName() + ": " + tool->getDescription() + "\n";
        }
        catch (const std::exception& e) {
            result += "- " + toolName + ": Error loading description: " + e.what() + "\n";
        }
    }
    return result;
}

void ToolRegistry::registerToolFactory(const std::string& toolName, std::function<std::unique_ptr<Tool>()> factory) {
    factories[toolName] = factory;
}