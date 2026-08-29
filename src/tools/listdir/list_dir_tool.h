#pragma once
#include "tools/tool.h"

class ListDirTool : public Tool {
public:
    ListDirTool();
    std::string execute(const std::string& args) override;
};
