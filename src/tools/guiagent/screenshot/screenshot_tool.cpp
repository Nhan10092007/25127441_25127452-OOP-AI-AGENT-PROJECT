#include "screenshot_tool.h"
#include "helper/encodeBase64.h"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

ScreenshotTool::ScreenshotTool()
    : Tool("capture_screenshot",
           "Captures a screenshot of the current desktop screen. "
           "Optional JSON parameter: 'filename' (e.g., 'screen.png'). "
           "Returns status and the Base64-encoded image data.")
{
    executor = ScreenshotExecutorFactory::createExecutor();
}

std::string ScreenshotTool::execute(const std::string& args) {
    std::string filename = "current_screen.png";
    if (!args.empty() && json::accept(args)) {
        json parsed = json::parse(args);
        if (parsed.contains("filename") && parsed["filename"].is_string()) {
            filename = parsed["filename"].get<std::string>();
        }
    }

    if (!executor->capture(filename)) {
        return json{{"status", "error"}, {"message", "Failed to capture screenshot."}}.dump();
    }

    std::string base64Data = encodeBase64(filename);
    if (base64Data.empty()) {
        return json{{"status", "error"}, {"message", "Failed to encode screenshot to Base64."}}.dump();
    }

    return json{
        {"status", "success"},
        {"message", "Screenshot captured successfully."},
        {"image_base64", base64Data}
    }.dump();
}
