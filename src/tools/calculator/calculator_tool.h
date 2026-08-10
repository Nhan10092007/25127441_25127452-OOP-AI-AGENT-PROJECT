#pragma once
#include "tools/tool.h"
#include <optional>
class CalculatorTool : public Tool {
public:
    CalculatorTool();
    std::string execute(const std::string& args) override;
};