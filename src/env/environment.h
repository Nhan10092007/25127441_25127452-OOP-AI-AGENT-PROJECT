#pragma once

#include<string>

struct EnvironmentConfig{
    std::string mode;
    std::string workspace;
};

class Environment{
public:
    
    virtual ~Environment() = default;
};