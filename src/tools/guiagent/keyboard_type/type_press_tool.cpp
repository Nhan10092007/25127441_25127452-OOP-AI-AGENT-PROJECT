#include "type_press_tool.h"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

KeyboardTypeTool::KeyboardTypeTool()
    : Tool("type_text",
           "Types a string of text into whatever UI element currently has keyboard focus "
           "(click on the target field first, or open it with key_press). "
           "Mandatory JSON parameter: 'text' (string). Example: {\"text\": \"Hello\"}")
{
    executor = KeyboardFactory::createExecutor();
}

std::string KeyboardTypeTool::execute(const std::string& args) {
    if (args.empty()) {
        return json{{"status", "error"}, {"message", "Missing arguments"}}.dump();
    }
    if (!json::accept(args)) {
        return json{{"status", "error"}, {"message", "Invalid JSON format"}}.dump();
    }

    json parsed = json::parse(args);
    if (!parsed.contains("text") || !parsed["text"].is_string()) {
        return json{{"status", "error"}, {"message", "Missing or invalid 'text' parameter"}}.dump();
    }

    std::string text = parsed["text"].get<std::string>();
    if (executor->typeText(text)) {
        return json{{"status", "success"}, {"message", "Typed text successfully"}}.dump();
    }
    return json{{"status", "error"}, {"message", "OS keyboard execution failed"}}.dump();
}