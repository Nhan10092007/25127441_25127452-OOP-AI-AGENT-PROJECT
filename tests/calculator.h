#pragma once
#include "Tool.h"

class CalculatorTool : public Tool {
public:
    CalculatorTool();
    std::optional<std::string> execute(const nlohmann::json& args) override;
};