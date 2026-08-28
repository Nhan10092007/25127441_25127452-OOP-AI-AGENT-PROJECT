#include "screenshot_capture.h"
#include "image_info.h"
#include <cstdio>
#include <cctype>
#include <cstdlib>
#include <filesystem>
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
        if (c == '.' || c == '_' || c == '-' || c == '/' || c == '\\' || c == ' ') continue;
        return false;
    }
    if (path.find("..") != std::string::npos) return false;
    return true;
}

static bool runCommand(const std::string& command) {
    FILE* pipe = POPEN(command.c_str(), "r");
    if (!pipe) {
        throw std::runtime_error("Failed to execute command: " + command);
    }
    char buffer[256];
    while (std::fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        // Đọc hết output để tiến trình con không bị nghẽn khi ghi ra pipe
    }
    return (PCLOSE(pipe) == 0);
}

// Đọc một dòng output đầu tiên của command (dùng để hỏi macOS về kích thước màn hình)
static std::string readCommandOutput(const std::string& command) {
    FILE* pipe = POPEN(command.c_str(), "r");
    if (!pipe) {
        return "";
    }
    std::string output;
    char buffer[256];
    while (std::fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        output += buffer;
    }
    PCLOSE(pipe);
    return output;
}

macOSScreenshotExecutor::macOSScreenshotExecutor()
    : IScreenshotExecutor("macOSScreenshotExecutor", "macOS Screenshot Executor") {}

WindowsScreenshotExecutor::WindowsScreenshotExecutor()
    : IScreenshotExecutor("WindowsScreenshotExecutor", "Windows Screenshot Executor") {}

LinuxScreenshotExecutor::LinuxScreenshotExecutor()
    : IScreenshotExecutor("LinuxScreenshotExecutor", "Linux Screenshot Executor") {}

// Trên macOS, `screencapture` xuất ảnh theo pixel vật lý (màn Retina = 2x),
// trong khi `cliclick` (tool click chuột) lại làm việc theo toạ độ logic (point).
// Nếu không chuẩn hoá, VLM nhìn ảnh 2880x1800 rồi click vào toạ độ đó => lệch gấp đôi.
// Vì vậy sau khi chụp, ta resize ảnh về đúng kích thước logic của màn hình bằng `sips`.
static bool macLogicalScreenSize(int& width, int& height) {
    // Finder trả về bounds của desktop theo point, ví dụ: "0, 0, 1440, 900"
    std::string output = readCommandOutput(
        "osascript -e 'tell application \"Finder\" to get bounds of window of desktop' 2>/dev/null");
    if (output.empty()) {
        return false;
    }
    int values[4] = {0, 0, 0, 0};
    int count = 0;
    std::size_t index = 0;
    while (index < output.size() && count < 4) {
        if (std::isdigit(static_cast<unsigned char>(output[index]))) {
            int number = 0;
            while (index < output.size() && std::isdigit(static_cast<unsigned char>(output[index]))) {
                number = number * 10 + (output[index] - '0');
                ++index;
            }
            values[count++] = number;
        } else {
            ++index;
        }
    }
    if (count < 4) {
        return false;
    }
    width  = values[2] - values[0];
    height = values[3] - values[1];
    return (width > 0 && height > 0);
}

bool macOSScreenshotExecutor::capture(const std::string& outputPath) {
    if (!isSafePath(outputPath)) {
        throw std::invalid_argument("Invalid or unsafe output path");
    }

    std::error_code errorCode;
    std::filesystem::path target(outputPath);
    if (target.has_parent_path()) {
        std::filesystem::create_directories(target.parent_path(), errorCode);
    }
    std::filesystem::remove(target, errorCode);

    if (!runCommand("screencapture -x \"" + outputPath + "\" 2>/dev/null")) {
        return false;
    }
    // screencapture vẫn trả về 0 khi thiếu quyền Screen Recording => kiểm tra file thật sự tồn tại
    if (!std::filesystem::exists(target, errorCode)) {
        return false;
    }

    int pixelWidth = 0, pixelHeight = 0;
    int logicalWidth = 0, logicalHeight = 0;
    if (readPngSize(outputPath, pixelWidth, pixelHeight) &&
        macLogicalScreenSize(logicalWidth, logicalHeight) &&
        (pixelWidth != logicalWidth || pixelHeight != logicalHeight)) {
        // sips -z <height> <width>
        runCommand("sips -z " + std::to_string(logicalHeight) + " " + std::to_string(logicalWidth) +
                   " \"" + outputPath + "\" 2>/dev/null");
    }
    return true;
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
    if (!runCommand(command)) {
        return false;
    }
    std::error_code errorCode;
    return std::filesystem::exists(outputPath, errorCode);
}

bool LinuxScreenshotExecutor::capture(const std::string& outputPath) {
    if (!isSafePath(outputPath)) {
        throw std::invalid_argument("Invalid or unsafe output path");
    }

    std::error_code errorCode;
    std::filesystem::path target(outputPath);
    if (target.has_parent_path()) {
        std::filesystem::create_directories(target.parent_path(), errorCode);
    }
    std::filesystem::remove(target, errorCode);

    std::string command =
        "gnome-screenshot -f \"" + outputPath + "\" 2>/dev/null || "
        "scrot -o \""            + outputPath + "\" 2>/dev/null || "
        "import -window root \"" + outputPath + "\" 2>/dev/null";
    if (!runCommand(command)) {
        return false;
    }
    return std::filesystem::exists(target, errorCode);
}

std::string macOSScreenshotExecutor::setupHint() const {
    return "On macOS, grant the Screen Recording permission to the terminal running this agent "
           "(System Settings > Privacy & Security > Screen Recording), then restart the terminal.";
}

std::string WindowsScreenshotExecutor::setupHint() const {
    return "On Windows, make sure PowerShell is available in PATH and the session has access to an interactive desktop.";
}

std::string LinuxScreenshotExecutor::setupHint() const {
    return "On Linux, install one of: gnome-screenshot, scrot or imagemagick (import), "
           "and make sure a graphical session (X11) is available.";
}
