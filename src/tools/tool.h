#pragma once
#include <string>
#include <optional>
#include "/Users/crocsa/Desktop/25127441_25127452-OOP-AI-AGENT-PROJECT/include/nlohmann/json.hpp"

class Tool {
    protected:
        std::string name;
        std::string description;
    public:
        virtual ~Tool() = default;
        Tool(std::string name, std::string description) : name(name), description(description) {}
        std::string getName() const { return name; }
        std::string getDescription() const { return description; }
        virtual std::string execute(const std::string& args) = 0;
};

