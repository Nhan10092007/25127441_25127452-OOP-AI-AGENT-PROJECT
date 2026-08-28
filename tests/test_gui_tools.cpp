// Smoke test cho GUI Agent: chạy trực tiếp 4 tool GUI mà không cần LLM.
// Dùng để kiểm tra môi trường (quyền Screen Recording / Accessibility trên macOS,
// cliclick đã cài chưa, xdotool trên Linux) trước khi chạy agent_runner.
//
//   ./gui_check                 -> chỉ chụp màn hình
//   ./gui_check --interactive   -> chụp màn hình + click + gõ chữ (sẽ điều khiển chuột/bàn phím thật)

#include "tools/guiagent/screenshot/screenshot_tool.h"
#include "tools/guiagent/mouse_click/mouse_click_tool.h"
#include "tools/guiagent/keyboard_type/type_press_tool.h"
#include "tools/guiagent/keyboard_type/key_press_tool.h"
#include "nlohmann/json.hpp"
#include <iostream>
#include <string>
#include <memory>

using json = nlohmann::json;

static bool runTool(const std::string& label, Tool& tool, const std::string& args) {
    std::cout << "\n=== " << label << " ===\n";
    std::cout << "args: " << (args.empty() ? "{}" : args) << "\n";
    try {
        std::string raw = tool.execute(args);
        json result = json::parse(raw);
        // Không in nguyên base64 ra terminal
        if (result.contains("image_base64")) {
            std::size_t length = result["image_base64"].get<std::string>().size();
            result["image_base64"] = "<" + std::to_string(length) + " base64 chars>";
        }
        std::cout << "result: " << result.dump(2) << "\n";
        return result.value("status", "") == "success";
    }
    catch (const std::exception& e) {
        std::cout << "EXCEPTION: " << e.what() << "\n";
        return false;
    }
}

int main(int argc, char** argv) {
    bool interactive = (argc > 1 && std::string(argv[1]) == "--interactive");

    int failed = 0;

    ScreenshotTool screenshot;
    if (!runTool("capture_screenshot", screenshot, R"({"filename":"gui_check.png"})")) {
        ++failed;
    }

    if (!interactive) {
        std::cout << "\nRun with --interactive to also test click / type_text / key_press "
                     "(these will move your real mouse and keyboard).\n";
        std::cout << (failed == 0 ? "\nSCREENSHOT OK\n" : "\nSCREENSHOT FAILED\n");
        return failed == 0 ? 0 : 1;
    }

    MouseClickTool click;
    if (!runTool("click", click, R"({"x":400,"y":400})")) {
        ++failed;
    }

    KeyboardTypeTool typeText;
    if (!runTool("type_text", typeText, R"({"text":"Hello from AI Agent"})")) {
        ++failed;
    }

    KeyPressTool keyPress;
    if (!runTool("key_press", keyPress, R"({"key":"Escape"})")) {
        ++failed;
    }

    std::cout << "\n" << (failed == 0 ? "ALL GUI TOOLS OK" : std::to_string(failed) + " GUI TOOL(S) FAILED") << "\n";
    return failed == 0 ? 0 : 1;
}
