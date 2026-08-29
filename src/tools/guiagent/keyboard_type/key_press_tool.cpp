#include "key_press_tool.h"
#include "nlohmann/json.hpp"
#include <string>

using json = nlohmann::json;

namespace {

#if defined(_WIN32) || defined(_WIN64)
constexpr const char* KEY_PRESS_OS_HINT =
    "Shortcuts are written with '+': win+r (Run dialog), win+s (Search), ctrl+n, ctrl+s, ctrl+c, "
    "alt+tab, alt+f4, shift+tab. "
    "Modifiers: ctrl, alt, shift, win (the Windows key; 'cmd' is accepted as an alias for it). "
    "Use ctrl - NOT cmd - for the usual copy/paste/save shortcuts on Windows. "
    "Mandatory JSON parameter: 'key' (string). Example: {\"key\": \"win+r\"}";
#elif defined(__APPLE__)
constexpr const char* KEY_PRESS_OS_HINT =
    "Shortcuts are written with '+': cmd+space (Spotlight), cmd+n, cmd+s, cmd+q, cmd+c, shift+tab. "
    "Modifiers: cmd (Command), ctrl, alt (Option), shift. "
    "Use cmd - NOT ctrl - for the usual copy/paste/save shortcuts on macOS. "
    "Mandatory JSON parameter: 'key' (string). Example: {\"key\": \"cmd+space\"}";
#else
constexpr const char* KEY_PRESS_OS_HINT =
    "Shortcuts are written with '+': ctrl+n, ctrl+s, ctrl+c, alt+tab, alt+f4, shift+tab, super+a. "
    "Modifiers: ctrl, alt, shift, super (the Super/Meta key; 'cmd' is accepted as an alias for it). "
    "Use ctrl - NOT cmd - for the usual copy/paste/save shortcuts on Linux. "
    "Mandatory JSON parameter: 'key' (string). Example: {\"key\": \"ctrl+s\"}";
#endif

} // namespace

KeyPressTool::KeyPressTool()
    : Tool("key_press",
           std::string(
               "Presses ONE functional key or ONE keyboard shortcut (NOT for typing text - use type_text for that). "
               "Single keys: Enter, Tab, Space, Escape, Backspace, Delete, Up, Down, Left, Right, Home, End, F1-F12. ")
               + KEY_PRESS_OS_HINT)
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