#pragma once
#include "./tools/tool.h"
#include "helper/keyboardFactory.h"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

class KeyboardTypeTool : public Tool {
private:
    std::unique_ptr<IKeyboardExecutor> executor;
public:
    KeyboardTypeTool();
    std::string execute(const std::string& args) override;
};