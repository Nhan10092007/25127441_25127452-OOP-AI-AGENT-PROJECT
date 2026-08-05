#pragma once
#include <memory>
#include <stdexcept>
#include "mouse_executor.h"

class MouseFactory {
public:
    static std::unique_ptr<IMouseExecutor> createExecutor() {
#if defined(__APPLE__)
        return std::make_unique<MacMouseExecutor>();
#elif defined(_WIN32) || defined(_WIN64)
        return std::make_unique<WindowsMouseExecutor>();
#elif defined(__linux__)
        return std::make_unique<LinuxMouseExecutor>();
#else
        throw std::runtime_error("Unsupported OS for Mouse actions.");
#endif
    }
};