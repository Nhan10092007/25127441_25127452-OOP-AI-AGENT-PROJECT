#pragma once
#include "tools/tool.h"
#include "helper/keyboardFactory.h"

class KeyboardTypeTool : public Tool {
private:
    std::unique_ptr<IKeyboardExecutor> executor;
public:
    KeyboardTypeTool();
    std::string execute(const std::string& args) override;
};