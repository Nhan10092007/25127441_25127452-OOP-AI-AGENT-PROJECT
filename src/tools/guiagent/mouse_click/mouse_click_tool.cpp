#include "mouse_click_tool.h"
MouseClickTool::MouseClickTool() : Tool("mouse_click",
                                        "Simulates a mouse click at the specified screen coordinates. "
                                        "Requires JSON parameters: 'x' and 'y' (e.g., '{\"x\": 100, \"y\": 200}'). "
                                        "Returns status and a message indicating success or failure.")
{
    executor = MouseFactory::createExecutor();
};

std::string MouseClickTool::execute(const std::string &args)
{
    if (args.empty())
    {
        return json{{"status", "error"}, {"message", "Missing arguments"}}.dump();
    }

    if (!json::accept(args))
    {
        return json{{"status", "error"}, {"message", "Invalid JSON format"}}.dump();
    }
    json parsed = json::parse(args);
    if (!parsed.contains("x") || !parsed.contains("y"))
    {
        return json{{"status", "error"}, {"message", "Missing 'x' or 'y'"}}.dump();
    }
    if (!parsed["x"].is_number() || !parsed["y"].is_number())
    {
        return json{{"status", "error"}, {"message", "'x' and 'y' must be numbers"}}.dump();
    }
    int x = parsed["x"].get<int>();
    int y = parsed["y"].get<int>();
    if (executor->click(x, y))
    {
        return json{{"status", "success"}, {"message", "Clicked at " + std::to_string(x) + ", " + std::to_string(y)}}.dump();
    }
    return json{{"status", "error"}, {"message", "OS mouse execution failed."}}.dump();
}
