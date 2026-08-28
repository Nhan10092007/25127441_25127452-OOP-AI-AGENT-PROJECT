#include "key_press_tool.h"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

KeyPressTool::KeyPressTool()
    : Tool("key_press",
           "Presses a single functional key (NOT for typing text - use type_text for that). "
           "Common keys: Enter, Tab, Escape, Backspace, Up, Down, Left, Right, Space. "
           "Mandatory JSON parameter: 'key' (string).")
{
    executor = KeyboardFactory::createExecutor();
}

std::string KeyPressTool::execute(const std::string& args) {
    if (args.empty()) {
        return json{{"status", "error"}, {"message", "Missing arguments"}}.dump();
    }
    if (!json::accept(args)) {
        return json{{"status", "error"}, {"message", "Invalid JSON format"}}.dump();
    }

    json parsed = json::parse(args);
    if (!parsed.contains("key") || !parsed["key"].is_string()) {
        return json{{"status", "error"}, {"message", "Missing or invalid 'key' parameter"}}.dump();
    }

    std::string key = parsed["key"].get<std::string>();
    if (executor->keyPress(key)) {
        return json{{"status", "success"}, {"message", "Pressed key: " + key}}.dump();
    }
    return json{{"status", "error"}, {"message", "OS keyboard execution failed, or unrecognized key name: " + key}}.dump();
}