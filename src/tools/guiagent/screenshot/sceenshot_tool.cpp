#include "sceenshot_tool.h"
#include "helper/encodeBase64.cpp"
ScreenshotTool::ScreenshotTool():Tool("capture_screenshot", 
               "Captures a screenshot of the current desktop screen. "
               "Optional JSON parameter: 'filename' (e.g., 'screen.png'). "
               "Returns status and the Base64-encoded image data.")
 {
    executor = ScreenshotExecutorFactory::createExecutor();
}

std::string ScreenshotTool::execute(const std::string& args) {
    std::string filename = "current_screen.png"; // Giá trị mặc định
    if (!args.empty()) {
        if (json::accept(args)) {
            json parsed = json::parse(args);
            if (parsed.contains("filename") && parsed["filename"].is_string()) {
                filename = parsed["filename"].get<std::string>();
            }
        }
    }
    bool success = executor->capture(filename);
    if (!success) {
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