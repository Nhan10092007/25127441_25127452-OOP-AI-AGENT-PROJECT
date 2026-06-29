#include "tool_registry.h"
#include "calculator.h"
#include "exec_tool.h"
#include "file_tool.h"
#include "web_tool.h"
#include "memory_tool.h"   // giả định: MemorySaveTool / MemorySearchTool theo thiết kế mới nhất

#include <iostream>
#include <string>

namespace {

// Chạy 1 test case: lấy tool theo tên từ registry, gọi execute(args),
// in kết quả nếu thành công, hoặc in message nếu throw exception.
void runTest(ToolRegistry& registry,
             const std::string& toolName,
             const std::string& args,
             const std::string& caseLabel) {
    std::cout << "----- [" << toolName << "] " << caseLabel << " -----\n";
    std::cout << "args: " << args << "\n";

    auto tool = registry.createTool(toolName);
    if (!tool) {
        std::cout << "  !! Tool not found in registry: " << toolName << "\n\n";
        return;
    }

    try {
        std::string result = tool->execute(args);
        std::cout << "  -> result: " << result << "\n\n";
    } catch (const std::exception& e) {
        std::cout << "  -> threw exception: " << e.what() << "\n\n";
    }
}

} // namespace

int main() {
    ToolRegistry registry;
    registry.registerTool<CalculatorTool>("calculator");
    registry.registerTool<ExecTool>("exec");
    registry.registerTool<ReadTool>("read_file");
    registry.registerTool<WriteTool>("write_file");
    registry.registerTool<WebTool>("web_search");
    // registry.registerTool<MemorySaveTool>("memory_save");
    // registry.registerTool<MemorySearchTool>("memory_search");

    // ================= CalculatorTool =================
    runTest(registry, "calculator", "2 + 3",          "basic add");
    runTest(registry, "calculator", "10 / 2 - 3",      "chain ops");
    runTest(registry, "calculator", "10 / 0",          "divide by zero -> throw");
    runTest(registry, "calculator", "abc",             "invalid start -> throw");
    runTest(registry, "calculator", "5 + ",            "missing operand -> throw");
    runTest(registry, "calculator", "",                "empty args -> throw");

    // ================= ExecTool =================
    runTest(registry, "exec", "echo hello",                  "basic echo");
    runTest(registry, "exec", "ls",                          "list dir");
    runTest(registry, "exec", "pwd",                         "print cwd");
    runTest(registry, "exec", "command_khong_ton_tai_xyz",   "unknown command");
    runTest(registry, "exec", "",                            "empty args -> throw");

    // ================= WriteTool / ReadTool =================
    // Chạy write trước để read có file thật mà đọc lại (test tích hợp 2 tool).
    runTest(registry, "write_file",
            R"({"path": "test_output.txt", "content": "Hello AI Agent"})",
            "write basic");
    runTest(registry, "write_file",
            R"({"path": "data/nested.txt", "content": "abc"})",
            "write into missing dir -> throw");
    runTest(registry, "write_file",
            R"({"path": "test_output.txt"})",
            "missing content -> throw");
    runTest(registry, "write_file", "not a json", "invalid json -> throw");

    runTest(registry, "read_file", "test_output.txt",        "read back written file");
    runTest(registry, "read_file", "file_khong_ton_tai.txt", "read missing file -> throw");
    runTest(registry, "read_file", "",                       "empty args -> throw");

    // ================= WebTool =================
    // Cần mạng thật — DuckDuckGo Instant Answer API.
    runTest(registry, "web_search", "Albert Einstein",   "normal query");
    runTest(registry, "web_search", "asdkjqwoieqwoiu",   "nonsense query (most fields empty)");
    runTest(registry, "web_search", "2+2",               "math-like query");
    runTest(registry, "web_search", "",                  "empty args -> throw");

    // ================= MemorySaveTool / MemorySearchTool =================
    // Lưu ý: nếu project vẫn đang dùng bản MemoryTool cũ (1 class, dispatch theo
    // field "action" trong JSON) thay vì 2 class MemorySaveTool/MemorySearchTool,
    // cần đổi lại tên đăng ký registry và format args cho khớp.
    runTest(registry, "memory_save",
            R"({"topic": "step1_sum", "value": "32"})",
            "save first time");
    runTest(registry, "memory_save",
            R"({"topic": "step1_sum", "value": "99"})",
            "overwrite same topic (expect INSERT OR REPLACE)");
    runTest(registry, "memory_save",
            R"({"topic": "step1_sum"})",
            "missing 'value' -> throw");
    runTest(registry, "memory_save", "not a json", "invalid json -> throw");

    runTest(registry, "memory_search", "step1_sum",
            "search existing topic (expect overwritten value '99', not '32')");
    runTest(registry, "memory_search", "topic_chua_tung_luu",
            "search missing topic (expect 'No memory found...' string, NOT a throw)");
    runTest(registry, "memory_search", "", "empty args -> throw");

    std::cout << "==== DONE ====" << std::endl;
    return 0;
}