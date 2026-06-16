#pragma once
#include "Tool.h"

class FileTool : public Tool {
public:
    FileTool();
    std::optional<std::string> execute(const nlohmann::json& args) override;
};