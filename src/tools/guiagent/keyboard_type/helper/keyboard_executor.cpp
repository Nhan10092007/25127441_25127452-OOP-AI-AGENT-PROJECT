#include "keyboard_executor.h"
#include <cstdlib>
#include <iostream>

#if defined(_WIN32) || defined(_WIN64)
    #include <windows.h>
#endif

bool LinuxKeyboardExecutor::typeText(const std::string& text) {
    std::string cmd = "xdotool type \"" + text + "\"";
    return (std::system(cmd.c_str()) == 0);
}
bool LinuxKeyboardExecutor::keyPress(const std::string& key) {
    std::string cmd = "xdotool key " + key;
    return (std::system(cmd.c_str()) == 0);
}


bool MacKeyboardExecutor::typeText(const std::string& text) {
    std::string cmd = "osascript -e 'tell application \"System Events\" to keystroke \"" + text + "\"'";
    return (std::system(cmd.c_str()) == 0);
}
bool MacKeyboardExecutor::keyPress(const std::string& key) {
    std::string cmd = "osascript -e 'tell application \"System Events\" to key code 36'"; 
    return (std::system(cmd.c_str()) == 0);
}

bool WindowsKeyboardExecutor::typeText(const std::string& text) {
#if defined(_WIN32) || defined(_WIN64)
    for (char c : text) {
        INPUT input = {0};
        input.type = INPUT_KEYBOARD;
        input.ki.wScan = c;                           
        input.ki.time = 0;
        input.ki.dwExtraInfo = 0;
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
    int vkCode = 0;
    if (key == "Return" || key == "Enter" || key == "enter") vkCode = VK_RETURN;
    else if (key == "Tab" || key == "tab") vkCode = VK_TAB;
    else if (key == "Space" || key == "space") vkCode = VK_SPACE;
    else if (key == "Backspace" || key == "backspace") vkCode = VK_BACK;
    else if (key == "Escape" || key == "esc") vkCode = VK_ESCAPE;
    else if (key == "Up" || key == "up") vkCode = VK_UP;
    else if (key == "Down" || key == "down") vkCode = VK_DOWN;
    
    if (vkCode != 0) {
        INPUT input = {0};
        input.type = INPUT_KEYBOARD;
        input.ki.wVk = vkCode; 
        SendInput(1, &input, sizeof(INPUT));
        input.ki.dwFlags = KEYEVENTF_KEYUP;
        SendInput(1, &input, sizeof(INPUT));
        
        return true;
    }
    return false;
#else
    return false;
#endif
}