#pragma once
#include <string>

class IKeyboardExecutor {
public:
    virtual ~IKeyboardExecutor() = default;
    virtual bool typeText(const std::string& text) = 0;
    virtual bool keyPress(const std::string& key) = 0;
};
class MacKeyboardExecutor : public IKeyboardExecutor {
public:
    bool typeText(const std::string& text) override;
    bool keyPress(const std::string& key) override;
};
class WindowsKeyboardExecutor : public IKeyboardExecutor {
public:
    bool typeText(const std::string& text) override;
    bool keyPress(const std::string& key) override;
};
class LinuxKeyboardExecutor : public IKeyboardExecutor {
public:
    bool typeText(const std::string& text) override;
    bool keyPress(const std::string& key) override;
}; 