#pragma once
#include "tool.h"

class ExecTool : public Tool {
public:
    ExecTool();
    std::optional<std::string> execute(const nlohmann::json& args) override;
};