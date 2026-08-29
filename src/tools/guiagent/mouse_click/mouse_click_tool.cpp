#include "mouse_click_tool.h"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

MouseClickTool::MouseClickTool()
    : Tool("click",
           "Moves the mouse and performs a left click at the given screen coordinates. "
           "Mandatory JSON parameters: 'x' (int), 'y' (int).")
{
    executor = MouseFactory::createExecutor();
}

std::string MouseClickTool::execute(const std::string& args) {
    if (args.empty()) {
        return json{{"status", "error"}, {"message", "Missing arguments"}}.dump();
    }
    if (!json::accept(args)) {
        return json{{"status", "error"}, {"message", "Invalid JSON format"}}.dump();
    }

    json parsed = json::parse(args);
    if (!parsed.contains("x") || !parsed["x"].is_number_integer()) {
        return json{{"status", "error"}, {"message", "Missing or invalid 'x' parameter"}}.dump();
    }
    if (!parsed.contains("y") || !parsed["y"].is_number_integer()) {
        return json{{"status", "error"}, {"message", "Missing or invalid 'y' parameter"}}.dump();
    }

    int x = parsed["x"].get<int>();
    int y = parsed["y"].get<int>();

    if (executor->click(x, y)) {
        return json{
            {"status", "success"},
            {"message", "Clicked at (" + std::to_string(x) + ", " + std::to_string(y) + ")"}
        }.dump();
    }
    return json{{"status", "error"}, {"message", "OS mouse execution failed"}}.dump();
}
