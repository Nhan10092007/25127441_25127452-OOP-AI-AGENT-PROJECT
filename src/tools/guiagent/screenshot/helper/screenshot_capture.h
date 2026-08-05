#pragma once
#include <string>
#include "nlohmann/json.hpp"


class IScreenshotExecutor{
private:
    std::string outputPath;
    std::string name;
    std::string description;
public:
    IScreenshotExecutor(std::string name, std::string description):name(name),description(description){};
    virtual ~IScreenshotExecutor()=default;
    virtual bool capture(const std::string&outputPath)=0;
};

class macOSScreenshotExecutor: public IScreenshotExecutor{
public:
    macOSScreenshotExecutor();
    bool capture(const std::string& outputPath) override;
};

class WindowsScreenshotExecutor: public IScreenshotExecutor{
public:
    WindowsScreenshotExecutor();
    bool capture(const std::string&outputPath) override;
};

class LinuxScreenshotExecutor: public IScreenshotExecutor{
public:
    LinuxScreenshotExecutor();
    bool capture(const std::string&outputPath) override;
};