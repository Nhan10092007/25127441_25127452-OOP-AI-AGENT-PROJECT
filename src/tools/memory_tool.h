#pragma once
#include "Tool.h"

class MemoryTool : public Tool {
public:
    MemoryTool();
    std::optional<std::string> execute(const nlohmann::json& args) override;
};