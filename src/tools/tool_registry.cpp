#include"tool_registry.h"

std::unique_ptr<Tool> ToolRegistry::createTool(const std::string& toolName) {
    auto it = factories.find(toolName);
    if (it != factories.end()) {
        return it->second(); 
    }
    return nullptr; 
}

std::string ToolRegistry::getToolsDescription(){
    std::string result = "## AVAILABLE TOOLS:\n";
    for(const auto& [toolName, factory] : factories){
        std::unique_ptr<Tool> tool = factory();
        result += "- " + tool->getName() + ": " + tool->getDescription() + "\n";
    }
    return result;
}