#pragma once
#include "tools/tool.h"

class StringTool : public Tool {
public:
    StringTool();
    std::string execute(const std::string& args) override;
};
