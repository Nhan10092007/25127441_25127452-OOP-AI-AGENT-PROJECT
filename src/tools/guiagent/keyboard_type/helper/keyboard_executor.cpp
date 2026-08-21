#include "keyboard_executor.h"
#include <cstdlib>
#include <unordered_map>
#include <algorithm>
#include <stdexcept>

#if defined(_WIN32) || defined(_WIN64)
    #include <windows.h>
#endif

static std::string sanitizeShellInput(const std::string& input) {
    std::string sanitized;
    sanitized.reserve(input.size());
    for (char c : input) {
        if (c == '\'' || c == '"' || c == '\\' || c == '`' ||
            c == '$' || c == '!' || c == '&' || c == '|' ||
            c == ';' || c == '\n' || c == '\r') {
            continue;
        }
        sanitized.push_back(c);
    }
    return sanitized;
}

bool LinuxKeyboardExecutor::typeText(const std::string& text) {
    std::string safe = sanitizeShellInput(text);
    std::string cmd = "xdotool type \"" + safe + "\"";
    return (std::system(cmd.c_str()) == 0);
}

bool LinuxKeyboardExecutor::keyPress(const std::string& key) {
    std::string safe = sanitizeShellInput(key);
    std::string cmd = "xdotool key " + safe;
    return (std::system(cmd.c_str()) == 0);
}

bool MacKeyboardExecutor::typeText(const std::string& text) {
    std::string safe = sanitizeShellInput(text);
    std::string cmd = "osascript -e 'tell application \"System Events\" to keystroke \""
                      + safe + "\"'";
    return (std::system(cmd.c_str()) == 0);
}

bool MacKeyboardExecutor::keyPress(const std::string& key) {
    static const std::unordered_map<std::string, int> keyCodeMap = {
        {"Return", 36}, {"Enter", 36}, {"enter", 36},
        {"Tab", 48},    {"tab", 48},
        {"Space", 49},  {"space", 49},
        {"Backspace", 51}, {"backspace", 51},
        {"Delete", 51},    {"delete", 51},
        {"Escape", 53}, {"Esc", 53}, {"esc", 53},
        {"Up", 126},    {"up", 126},
        {"Down", 125},  {"down", 125},
        {"Left", 123},  {"left", 123},
        {"Right", 124}, {"right", 124}
    };

    auto it = keyCodeMap.find(key);
    if (it == keyCodeMap.end()) {
        return false;
    }

    std::string cmd = "osascript -e 'tell application \"System Events\" to key code "
                      + std::to_string(it->second) + "'";
    return (std::system(cmd.c_str()) == 0);
}

bool WindowsKeyboardExecutor::typeText(const std::string& text) {
#if defined(_WIN32) || defined(_WIN64)
    for (char c : text) {
        INPUT input = {0};
        input.type = INPUT_KEYBOARD;
        input.ki.wScan = c;
        input.ki.wVk = 0;
        input.ki.dwFlags = KEYEVENTF_UNICODE;
        SendInput(1, &input, sizeof(INPUT));
        input.ki.dwFlags = KEYEVENTF_UNICODE | KEYEVENTF_KEYUP;
        SendInput(1, &input, sizeof(INPUT));
    }
    return true;
#else
    return false;
#endif
}

bool WindowsKeyboardExecutor::keyPress(const std::string& key) {
#if defined(_WIN32) || defined(_WIN64)
    static const std::unordered_map<std::string, int> vkMap = {
        {"Return", VK_RETURN}, {"Enter", VK_RETURN}, {"enter", VK_RETURN},
        {"Tab", VK_TAB},       {"tab", VK_TAB},
        {"Space", VK_SPACE},   {"space", VK_SPACE},
        {"Backspace", VK_BACK},{"backspace", VK_BACK},
        {"Escape", VK_ESCAPE}, {"Esc", VK_ESCAPE}, {"esc", VK_ESCAPE},
        {"Up", VK_UP},         {"up", VK_UP},
        {"Down", VK_DOWN},     {"down", VK_DOWN},
        {"Left", VK_LEFT},     {"left", VK_LEFT},
        {"Right", VK_RIGHT},   {"right", VK_RIGHT}
    };

    auto it = vkMap.find(key);
    if (it == vkMap.end()) {
        return false;
    }

    INPUT input = {0};
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = static_cast<WORD>(it->second);
    SendInput(1, &input, sizeof(INPUT));
    input.ki.dwFlags = KEYEVENTF_KEYUP;
    SendInput(1, &input, sizeof(INPUT));
    return true;
#else
    return false;
#endif
}