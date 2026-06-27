#pragma once
#include "tool.h"

class MemoryTool : public Tool {
public:
    MemoryTool();
   std::string execute(const std::string& args) override;
};
