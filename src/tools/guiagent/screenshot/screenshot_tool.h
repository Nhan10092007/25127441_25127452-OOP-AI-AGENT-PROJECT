#pragma once
#include "tools/tool.h"
#include "helper/screenshotFactory.h"

class ScreenshotTool : public Tool {
private:
    std::unique_ptr<IScreenshotExecutor> executor;
public:
    ScreenshotTool();
    std::string execute(const std::string& args) override;
};
