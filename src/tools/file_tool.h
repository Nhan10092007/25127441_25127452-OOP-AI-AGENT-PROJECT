#pragma once
#include "tool.h"

class ReadTool : public Tool {
public:
    ReadTool();
    std::string execute(const std::string& args) override;

};

class WriteTool : public Tool {
public:
    WriteTool();
    std::string execute(const std::string& args) override;
};