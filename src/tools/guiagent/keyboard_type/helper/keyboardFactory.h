#pragma once
#include <memory>
#include "keyboard_executor.h"

class KeyboardFactory {
public:
    static std::unique_ptr<IKeyboardExecutor> createExecutor() {
#if defined(__APPLE__)
        return std::make_unique<MacKeyboardExecutor>();
#elif defined(_WIN32) || defined(_WIN64)
        return std::make_unique<WindowsKeyboardExecutor>();
#elif defined(__linux__)
        return std::make_unique<LinuxKeyboardExecutor>();
#else
        throw std::runtime_error("Unsupported OS for Keyboard actions.");
#endif
    }
};