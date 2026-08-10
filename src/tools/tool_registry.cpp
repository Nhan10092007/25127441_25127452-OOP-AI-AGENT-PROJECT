#include"tool_registry.h"

std::unique_ptr<Tool> ToolRegistry::createTool(const std::string& toolName) const{
    auto it = factories.find(toolName);
    if (it != factories.end()) {
        return it->second(); 
    }
    return nullptr; 
}

std::string ToolRegistry::getToolsDescription() const{
    std::string result = "## AVAILABLE TOOLS:\n";
    for(const auto& [toolName, factory] : factories){
        try{
            std::unique_ptr<Tool> tool = factory();
            result += "- " + tool->getName() + ": " + tool->getDescription() + "\n";
        }
        catch(const std::exception& e){
            result += "- " + toolName + ": Error loading description: " + e.what() + "\n";
        }
    }
    return result;
}

void ToolRegistry::registerToolFactory(const std::string& toolName, std::function<std::unique_ptr<Tool>()> factory){
    factories[toolName] = factory;
}