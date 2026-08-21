#include "screenshot_capture.h"
#include <cstdio>
#include <stdexcept>
#include <string>
#include <algorithm>

#if defined(_WIN32) || defined(_WIN64)
    #define POPEN _popen
    #define PCLOSE _pclose
#else
    #define POPEN popen
    #define PCLOSE pclose
#endif

static bool isSafePath(const std::string& path) {
    if (path.empty()) return false;
    for (char c : path) {
        if (std::isalnum(static_cast<unsigned char>(c))) continue;
        if (c == '.' || c == '_' || c == '-' || c == '/' || c == '\\') continue;
        return false;
    }
    if (path.find("..") != std::string::npos) return false;
    return true;
}

macOSScreenshotExecutor::macOSScreenshotExecutor()
    : IScreenshotExecutor("macOSScreenshotExecutor", "macOS Screenshot Executor") {}

WindowsScreenshotExecutor::WindowsScreenshotExecutor()
    : IScreenshotExecutor("WindowsScreenshotExecutor", "Windows Screenshot Executor") {}

LinuxScreenshotExecutor::LinuxScreenshotExecutor()
    : IScreenshotExecutor("LinuxScreenshotExecutor", "Linux Screenshot Executor") {}

bool macOSScreenshotExecutor::capture(const std::string& outputPath) {
    if (!isSafePath(outputPath)) {
        throw std::invalid_argument("Invalid or unsafe output path");
    }
    std::string command = "screencapture -x \"" + outputPath + "\"";
    FILE* pipe = POPEN(command.c_str(), "r");
    if (!pipe) {
        throw std::runtime_error("Failed to execute command: " + command);
    }
    int returnCode = PCLOSE(pipe);
    return (returnCode == 0);
}

bool WindowsScreenshotExecutor::capture(const std::string& outputPath) {
    if (!isSafePath(outputPath)) {
        throw std::invalid_argument("Invalid or unsafe output path");
    }
    std::string command =
        "powershell -NoProfile -Command \""
        "Add-Type -AssemblyName System.Drawing, System.Windows.Forms; "
        "$b = [System.Drawing.Bitmap]::new("
            "[System.Windows.Forms.Screen]::PrimaryScreen.Bounds.Width, "
            "[System.Windows.Forms.Screen]::PrimaryScreen.Bounds.Height); "
        "$g = [System.Drawing.Graphics]::FromImage($b); "
        "$g.CopyFromScreen(0,0,0,0,$b.Size); "
        "$b.Save('" + outputPath + "')\"";
    FILE* pipe = POPEN(command.c_str(), "r");
    if (!pipe) {
        throw std::runtime_error("Failed to execute command: " + command);
    }
    int returnCode = PCLOSE(pipe);
    return (returnCode == 0);
}

bool LinuxScreenshotExecutor::capture(const std::string& outputPath) {
    if (!isSafePath(outputPath)) {
        throw std::invalid_argument("Invalid or unsafe output path");
    }
    std::string command =
        "gnome-screenshot -f \"" + outputPath + "\" 2>/dev/null || "
        "scrot \""               + outputPath + "\" 2>/dev/null || "
        "import -window root \"" + outputPath + "\" 2>/dev/null";
    FILE* pipe = POPEN(command.c_str(), "r");
    if (!pipe) {
        throw std::runtime_error("Failed to execute command: " + command);
    }
    int returnCode = PCLOSE(pipe);
    return (returnCode == 0);
}