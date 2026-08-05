#pragma once
#include "./tools/tool.h"
#include "helper/screenshotFactory.h"
#include "nlohmann/json.hpp"
using json = nlohmann::json;

class ScreenshotTool : public Tool {
private:
    std::unique_ptr<IScreenshotExecutor> executor;
public:
    ScreenshotTool();
    std::string execute(const std::string& args) override;
};