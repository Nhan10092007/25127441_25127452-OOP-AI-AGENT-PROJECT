#pragma once
#include <string>

class IScreenshotExecutor {
private:
    std::string name;
    std::string description;
public:
    IScreenshotExecutor(std::string name, std::string description)
        : name(std::move(name)), description(std::move(description)) {}
    virtual ~IScreenshotExecutor() = default;
    virtual bool capture(const std::string& outputPath) = 0;
    // Gợi ý cấu hình khi chụp màn hình thất bại (mỗi HĐH có yêu cầu quyền/phần mềm khác nhau)
    virtual std::string setupHint() const = 0;
};

class macOSScreenshotExecutor : public IScreenshotExecutor {
public:
    macOSScreenshotExecutor();
    bool capture(const std::string& outputPath) override;
    std::string setupHint() const override;
};

class WindowsScreenshotExecutor : public IScreenshotExecutor {
public:
    WindowsScreenshotExecutor();
    bool capture(const std::string& outputPath) override;
    std::string setupHint() const override;
};

class LinuxScreenshotExecutor : public IScreenshotExecutor {
public:
    LinuxScreenshotExecutor();
    bool capture(const std::string& outputPath) override;
    std::string setupHint() const override;
};