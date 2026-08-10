#pragma once
#include "./tools/tool.h"
#include "helper/mouseFactory.h"

class MouseClickTool : public Tool {
private:
    std::unique_ptr<IMouseExecutor> executor;
public:
    MouseClickTool();
    std::string execute(const std::string& args) override;
};
