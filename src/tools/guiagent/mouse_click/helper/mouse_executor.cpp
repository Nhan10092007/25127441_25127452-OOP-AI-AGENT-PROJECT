#include "mouse_executor.h"
#include "tools/guiagent/common/gui_utils.h"
#include <cstdlib>
#include <string>
#include <iostream>

#if defined(_WIN32) || defined(_WIN64)
    #include <windows.h>
#endif

bool LinuxMouseExecutor::click(int x, int y) {
    gui_utils::requireCommand("xdotool", "Install it with: sudo apt install xdotool (Debian/Ubuntu).");
    std::string cmd = "xdotool mousemove " + std::to_string(x) + " " + std::to_string(y) + " click 1";
    bool ok = (std::system(cmd.c_str()) == 0);
    if (ok) {
        gui_utils::waitForUi();
    }
    return ok;
}

bool MacMouseExecutor::click(int x, int y) {
    // cliclick không có sẵn trên macOS, phải cài qua Homebrew.
    // Toạ độ của cliclick là toạ độ logic (point) - trùng với ảnh screenshot đã được
    // chuẩn hoá về kích thước logic trong macOSScreenshotExecutor.
    gui_utils::requireCommand("cliclick", "Install it with: brew install cliclick.");
    std::string cmd = "cliclick c:" + std::to_string(x) + "," + std::to_string(y);
    bool ok = (std::system(cmd.c_str()) == 0);
    if (ok) {
        gui_utils::waitForUi();
    }
    return ok;
}

bool WindowsMouseExecutor::click(int x, int y) {
#if defined(_WIN32) || defined(_WIN64)
    SetCursorPos(x, y);
    mouse_event(MOUSEEVENTF_LEFTDOWN, x, y, 0, 0);
    mouse_event(MOUSEEVENTF_LEFTUP, x, y, 0, 0);
    gui_utils::waitForUi();
    return true;
#else
    std::cout << "[Windows Mock] Clicked at " << x << ", " << y << "\n";
    return true;
#endif
}
