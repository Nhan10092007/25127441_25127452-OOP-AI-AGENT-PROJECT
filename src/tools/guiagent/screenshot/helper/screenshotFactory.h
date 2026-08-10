#pragma once
#include <memory>
#include "screenshot_capture.h"

class ScreenshotExecutorFactory {
public:
    static std::unique_ptr<IScreenshotExecutor> createExecutor() {
#if defined(__APPLE__)
        return std::make_unique<macOSScreenshotExecutor>();
#elif defined(_WIN32) || defined(_WIN64)
        return std::make_unique<WindowsScreenshotExecutor>();
#elif defined(__linux__)
        return std::make_unique<LinuxScreenshotExecutor>();
#else
        throw std::runtime_error("Unsupported Operating System for Screenshot Execution.");
#endif
    }
};