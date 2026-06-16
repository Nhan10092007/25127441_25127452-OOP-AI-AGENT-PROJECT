#pragma once
#include "tool.h"
#include <optional>
class CalculatorTool : public Tool {
public:
    CalculatorTool();
    std::optional<std::string> execute(const nlohmann::json& args) override;
};