#pragma once

#include"environment.h"
#include"tools/tool_registry.h"
#include<memory>

class NativeEnvironment : public Environment{
private:
    std::shared_ptr<ToolRegistry> toolRegistry; 
public:
    NativeEnvironment(std::shared_ptr<ToolRegistry> registry);
    ToolResult step(const ToolInput& toolRequest) override;
};