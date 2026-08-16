#pragma once
#include <string>
#include "nlohmann/json.hpp"

class Tool {
protected:
    std::string name;
    std::string description;
public:
    virtual ~Tool() = default;
    Tool(std::string name, std::string description) : name(name), description(description) {}
    const std::string& getName() const {
        return name;
    }
    const std::string& getDescription() const { 
        return description; 
    }
    virtual std::string execute(const std::string& args) = 0;
};