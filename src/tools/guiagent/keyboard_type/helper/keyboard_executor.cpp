#include "keyboard_executor.h"
#include "tools/guiagent/common/gui_utils.h"
#include <cstdlib>
#include <unordered_map>
#include <vector>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <stdexcept>

#if defined(_WIN32) || defined(_WIN64)
    #include <windows.h>
#endif

namespace {

// Bọc chuỗi trong dấu nháy đơn để shell không diễn giải bất kỳ ký tự đặc biệt nào.
// Mọi ký tự ' bên trong được thay bằng '\'' (đóng nháy - nháy escape - mở nháy lại).
std::string shellQuote(const std::string& text) {
    std::string quoted = "'";
    for (char c : text) {
        if (c == '\'') {
            quoted += "'\\''";
        } else {
            quoted.push_back(c);
        }
    }
    quoted.push_back('\'');
    return quoted;
}

// Escape cho chuỗi literal của AppleScript: chỉ cần xử lý \ và "
std::string appleScriptQuote(const std::string& text) {
    std::string escaped;
    for (char c : text) {
        if (c == '\\' || c == '"') {
            escaped.push_back('\\');
            escaped.push_back(c);
        } else if (c == '\n') {
            escaped += "\\n";
        } else if (c == '\r') {
            continue;
        } else {
            escaped.push_back(c);
        }
    }
    return escaped;
}

std::string toLower(const std::string& text) {
    std::string lowered = text;
    for (char& c : lowered) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return lowered;
}

std::string trim(const std::string& text) {
    auto first = text.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    auto last = text.find_last_not_of(" \t\r\n");
    return text.substr(first, last - first + 1);
}

// Tách "cmd+shift+n" thành danh sách modifier {"cmd", "shift"} và phím chính "n".
// Hỗ trợ cả dấu '+' lẫn '-' làm ký tự nối.
bool splitCombo(const std::string& input, std::vector<std::string>& modifiers, std::string& mainKey) {
    modifiers.clear();
    mainKey.clear();

    std::vector<std::string> tokens;
    std::string current;
    for (char c : input) {
        if ((c == '+' || c == '-') && !trim(current).empty()) {
            tokens.push_back(trim(current));
            current.clear();
        } else {
            current.push_back(c);
        }
    }
    if (!trim(current).empty()) {
        tokens.push_back(trim(current));
    }
    if (tokens.empty()) {
        return false;
    }

    static const std::unordered_map<std::string, std::string> modifierAlias = {
        {"cmd", "cmd"},   {"command", "cmd"}, {"meta", "cmd"}, {"super", "cmd"}, {"win", "cmd"},
        {"ctrl", "ctrl"}, {"control", "ctrl"},
        {"alt", "alt"},   {"option", "alt"},  {"opt", "alt"},
        {"shift", "shift"}
    };

    for (std::size_t i = 0; i + 1 < tokens.size(); ++i) {
        auto it = modifierAlias.find(toLower(tokens[i]));
        if (it == modifierAlias.end()) {
            return false;  // token ở giữa không phải modifier hợp lệ
        }
        modifiers.push_back(it->second);
    }
    mainKey = tokens.back();
    return !mainKey.empty();
}

// Chạy lệnh rồi chờ UI cập nhật, để screenshot xác minh ở bước sau thấy được trạng thái mới.
bool runAndWait(const std::string& command) {
    if (std::system(command.c_str()) != 0) {
        return false;
    }
    gui_utils::waitForUi();
    return true;
}

} // namespace

// ------------------------------ Linux (xdotool) ------------------------------

bool LinuxKeyboardExecutor::typeText(const std::string& text) {
    if (text.empty()) {
        return false;
    }
    gui_utils::requireCommand("xdotool", "Install it with: sudo apt install xdotool (Debian/Ubuntu).");
    std::string cmd = "xdotool type --clearmodifiers -- " + shellQuote(text);
    return runAndWait(cmd);
}

bool LinuxKeyboardExecutor::keyPress(const std::string& key) {
    std::vector<std::string> modifiers;
    std::string mainKey;
    if (!splitCombo(key, modifiers, mainKey)) {
        return false;
    }

    // Tên phím chuẩn của X11 (xdotool phân biệt hoa thường)
    static const std::unordered_map<std::string, std::string> keyNameMap = {
        {"return", "Return"}, {"enter", "Return"},
        {"tab", "Tab"},       {"space", "space"},
        {"backspace", "BackSpace"}, {"delete", "Delete"},
        {"escape", "Escape"}, {"esc", "Escape"},
        {"up", "Up"}, {"down", "Down"}, {"left", "Left"}, {"right", "Right"},
        {"home", "Home"}, {"end", "End"},
        {"pageup", "Page_Up"}, {"pagedown", "Page_Down"}
    };
    static const std::unordered_map<std::string, std::string> modifierMap = {
        {"cmd", "super"}, {"ctrl", "ctrl"}, {"alt", "alt"}, {"shift", "shift"}
    };

    std::string keySequence;
    for (const std::string& modifier : modifiers) {
        keySequence += modifierMap.at(modifier) + "+";
    }

    auto it = keyNameMap.find(toLower(mainKey));
    if (it != keyNameMap.end()) {
        keySequence += it->second;
    } else if (mainKey.size() == 1 && std::isalnum(static_cast<unsigned char>(mainKey[0]))) {
        keySequence += mainKey;
    } else if (mainKey.size() >= 2 && (mainKey[0] == 'F' || mainKey[0] == 'f') &&
               std::isdigit(static_cast<unsigned char>(mainKey[1]))) {
        keySequence += "F" + mainKey.substr(1);
    } else {
        return false;
    }

    gui_utils::requireCommand("xdotool", "Install it with: sudo apt install xdotool (Debian/Ubuntu).");
    std::string cmd = "xdotool key --clearmodifiers -- " + shellQuote(keySequence);
    return runAndWait(cmd);
}

// ------------------------------ macOS (osascript / System Events) ------------------------------

bool MacKeyboardExecutor::typeText(const std::string& text) {
    if (text.empty()) {
        return false;
    }
    std::string script = "tell application \"System Events\" to keystroke \"" +
                         appleScriptQuote(text) + "\"";
    std::string cmd = "osascript -e " + shellQuote(script);
    return runAndWait(cmd);
}

bool MacKeyboardExecutor::keyPress(const std::string& key) {
    std::vector<std::string> modifiers;
    std::string mainKey;
    if (!splitCombo(key, modifiers, mainKey)) {
        return false;
    }

    // Virtual key code của macOS cho các phím chức năng
    static const std::unordered_map<std::string, int> keyCodeMap = {
        {"return", 36}, {"enter", 36},
        {"tab", 48},    {"space", 49},
        {"backspace", 51}, {"delete", 51},
        {"escape", 53}, {"esc", 53},
        {"up", 126}, {"down", 125}, {"left", 123}, {"right", 124},
        {"home", 115}, {"end", 119},
        {"pageup", 116}, {"pagedown", 121},
        {"f1", 122}, {"f2", 120}, {"f3", 99}, {"f4", 118}, {"f5", 96}, {"f6", 97},
        {"f7", 98},  {"f8", 100}, {"f9", 101}, {"f10", 109}, {"f11", 103}, {"f12", 111}
    };
    static const std::unordered_map<std::string, std::string> modifierMap = {
        {"cmd", "command down"}, {"ctrl", "control down"},
        {"alt", "option down"},  {"shift", "shift down"}
    };

    std::string usingClause;
    if (!modifiers.empty()) {
        usingClause = " using {";
        for (std::size_t i = 0; i < modifiers.size(); ++i) {
            if (i > 0) usingClause += ", ";
            usingClause += modifierMap.at(modifiers[i]);
        }
        usingClause += "}";
    }

    std::string action;
    auto it = keyCodeMap.find(toLower(mainKey));
    if (it != keyCodeMap.end()) {
        action = "key code " + std::to_string(it->second);
    } else if (mainKey.size() == 1) {
        // Phím ký tự đơn (ví dụ cmd+n, cmd+s) dùng keystroke thay vì key code
        action = "keystroke \"" + appleScriptQuote(mainKey) + "\"";
    } else {
        return false;
    }

    std::string script = "tell application \"System Events\" to " + action + usingClause;
    std::string cmd = "osascript -e " + shellQuote(script);
    return runAndWait(cmd);
}

// ------------------------------ Windows (SendInput) ------------------------------

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
    (void)text;
    return false;
#endif
}

bool WindowsKeyboardExecutor::keyPress(const std::string& key) {
#if defined(_WIN32) || defined(_WIN64)
    std::vector<std::string> modifiers;
    std::string mainKey;
    if (!splitCombo(key, modifiers, mainKey)) {
        return false;
    }

    static const std::unordered_map<std::string, int> vkMap = {
        {"return", VK_RETURN}, {"enter", VK_RETURN},
        {"tab", VK_TAB},       {"space", VK_SPACE},
        {"backspace", VK_BACK},{"delete", VK_DELETE},
        {"escape", VK_ESCAPE}, {"esc", VK_ESCAPE},
        {"up", VK_UP}, {"down", VK_DOWN}, {"left", VK_LEFT}, {"right", VK_RIGHT},
        {"home", VK_HOME}, {"end", VK_END},
        {"pageup", VK_PRIOR}, {"pagedown", VK_NEXT}
    };
    static const std::unordered_map<std::string, int> modifierVkMap = {
        {"cmd", VK_LWIN}, {"ctrl", VK_CONTROL}, {"alt", VK_MENU}, {"shift", VK_SHIFT}
    };

    int mainVk = 0;
    auto it = vkMap.find(toLower(mainKey));
    if (it != vkMap.end()) {
        mainVk = it->second;
    } else if (mainKey.size() == 1 && std::isalnum(static_cast<unsigned char>(mainKey[0]))) {
        mainVk = std::toupper(static_cast<unsigned char>(mainKey[0]));
    } else {
        return false;
    }

    auto sendKey = [](int virtualKey, bool keyUp) {
        INPUT input = {0};
        input.type = INPUT_KEYBOARD;
        input.ki.wVk = static_cast<WORD>(virtualKey);
        input.ki.dwFlags = keyUp ? KEYEVENTF_KEYUP : 0;
        SendInput(1, &input, sizeof(INPUT));
    };

    for (const std::string& modifier : modifiers) {
        sendKey(modifierVkMap.at(modifier), false);
    }
    sendKey(mainVk, false);
    sendKey(mainVk, true);
    for (auto rit = modifiers.rbegin(); rit != modifiers.rend(); ++rit) {
        sendKey(modifierVkMap.at(*rit), true);
    }
    return true;
#else
    (void)key;
    return false;
#endif
}
