#pragma once
#include "./tool.h"

class ReadFileTool : public Tool {
public:
    ReadFileTool();
    std::string execute(const std::string& args) override;

};

class WriteFileTool : public Tool {
public:
    WriteFileTool();
    std::string execute(const std::string& args) override;
};