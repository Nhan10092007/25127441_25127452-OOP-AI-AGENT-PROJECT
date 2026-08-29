#pragma once
#include "./tools/tool.h"
#include "helper/keyboardFactory.h"

class KeyPressTool : public Tool {
private:
    std::unique_ptr<IKeyboardExecutor> executor;
public:
    KeyPressTool();
    std::string execute(const std::string& args) override;
};