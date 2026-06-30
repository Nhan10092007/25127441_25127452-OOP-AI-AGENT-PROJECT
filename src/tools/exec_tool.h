#pragma once
#include "tool.h"

class ExecTool : public Tool {
public:
    ExecTool();
    std::string execute(const std::string& args) override;
};