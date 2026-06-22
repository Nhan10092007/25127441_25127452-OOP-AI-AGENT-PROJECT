#pragma once

#include "environment.h"
#include<memory>

class ToolRegistry;

class NativeEnvironment : public Environment{
private:
    std::shared_ptr<ToolRegistry> toolRegistry; 
public:
    NativeEnvironment(std::shared_ptr<ToolRegistry> registry);
    ToolResult step(const ToolInput& toolRequest) override;
};