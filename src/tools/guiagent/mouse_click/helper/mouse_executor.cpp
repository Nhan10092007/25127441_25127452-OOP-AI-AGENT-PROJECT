#include "mouse_executor.h"
#include <cstdlib>
#include <string>
#include <iostream>


#if defined(_WIN32) || defined(_WIN64)
    #include <windows.h>
#endif


bool LinuxMouseExecutor::click(int x, int y) {
    std::string cmd = "xdotool mousemove " + std::to_string(x) + " " + std::to_string(y) + " click 1";
    return (std::system(cmd.c_str()) == 0);
}


bool MacMouseExecutor::click(int x, int y) {
    std::string cmd = "cliclick c:" + std::to_string(x) + "," + std::to_string(y);
    return (std::system(cmd.c_str()) == 0);
}


bool WindowsMouseExecutor::click(int x, int y) {
#if defined(_WIN32) || defined(_WIN64)
    SetCursorPos(x, y);
    mouse_event(MOUSEEVENTF_LEFTDOWN, x, y, 0, 0);
    mouse_event(MOUSEEVENTF_LEFTUP, x, y, 0, 0);
    return true; 
#else
    std::cout << "[Windows Mock] Clicked at " << x << ", " << y << "\n";
    return true;
#endif
}