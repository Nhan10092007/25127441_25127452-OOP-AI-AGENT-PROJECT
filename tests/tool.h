#pragma once;
#include <string>
#include <optional>
#include <nlohmann/json.hpp>

class Tool {
    private:
        std::string name;
        std::string description;
    public:
        virtual ~Tool() = default;
        Tool(std::string name, std::string description) : name(name), description(description) {}
        std::string getName() const { return name; }
        std::string getDescription() const { return description; }
        virtual std::optional<std::string> execute(const nlohmann::json& args) = 0;
};

