#pragma once
#include "tools/tool.h"

class DatetimeTool : public Tool {
public:
    DatetimeTool();
    std::string execute(const std::string& args) override;
};
