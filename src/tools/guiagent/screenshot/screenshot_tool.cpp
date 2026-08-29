#include "screenshot_tool.h"
#include "helper/encodeBase64.h"
#include "helper/image_info.h"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

ScreenshotTool::ScreenshotTool()
    : Tool("capture_screenshot",
           "Captures a screenshot of the whole desktop screen. "
           "Optional JSON parameter: 'filename' (e.g., {\"filename\": \"screen.png\"}); "
           "call it with {} to use the default file. "
           "Returns the image itself plus 'width' and 'height': the (x, y) you pass to the "
           "'click' tool MUST be read from this image and stay inside 0..width and 0..height.")
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
        return json{
            {"status", "error"},
            {"message", "Failed to capture screenshot. " + executor->setupHint()}
        }.dump();
    }

    std::string base64Data = encodeBase64(filename);
    if (base64Data.empty()) {
        return json{{"status", "error"}, {"message", "Failed to encode screenshot to Base64."}}.dump();
    }

    int width = 0, height = 0;
    json result = {
        {"status", "success"},
        {"message", "Screenshot captured successfully."},
        {"image_base64", base64Data}
    };
    // Báo cho VLM biết hệ toạ độ của ảnh để nó ước lượng (x, y) cho tool click
    if (readPngSize(filename, width, height)) {
        result["width"] = width;
        result["height"] = height;
        result["message"] = "Screenshot captured successfully. Image coordinate space is " +
                            std::to_string(width) + "x" + std::to_string(height) +
                            " and matches the coordinates used by the 'click' tool.";
    }
    return result.dump();
}
