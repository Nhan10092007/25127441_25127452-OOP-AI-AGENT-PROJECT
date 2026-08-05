#pragma once

#include<string>
#include<optional>
struct EnvironmentConfig{
    std::string mode;
    std::string workspace;
};

struct ToolResult{ // Output
    std::string content;
    bool success;
};

struct ToolInput{
    std::string toolName;
    std::string args;
};

class Environment{
public:
    virtual ToolResult step(const ToolInput& toolRequest) = 0;
    virtual ~Environment() = default;
};