// Test VisionAgentLoop bằng LLM giả và Environment giả (không cần Ollama, không cần màn hình).
// Mục tiêu: chứng minh ảnh screenshot được đưa vào kênh 'images' của Message
// và lịch sử chỉ giữ lại ảnh mới nhất.

#include "agent/vision_agent_loop.h"
#include <iostream>
#include <string>
#include <vector>

using json = nlohmann::json;

namespace {

// LLM giả: trả về lần lượt các response đã soạn sẵn
class FakeLLMClient : public LLMClient {
private:
    std::vector<std::string> scriptedResponses;
    std::size_t index = 0;
public:
    std::vector<Message> lastMessages;

    explicit FakeLLMClient(std::vector<std::string> responses)
        : scriptedResponses(std::move(responses)) {}

    LLMResponse sendRequest(const std::vector<Message>& messages) override {
        lastMessages = messages;
        std::string response = (index < scriptedResponses.size())
            ? scriptedResponses[index++]
            : R"({"thought":"done","action":{"type":"finish","result":"success"}})";
        return LLMResponse{response, 10, 10, 20, true};
    }
};

// Environment giả: capture_screenshot trả về một "ảnh" base64 giả
class FakeEnvironment : public Environment {
public:
    ToolResult step(const ToolInput& toolRequest) override {
        if (toolRequest.toolName == "capture_screenshot") {
            json result = {
                {"status", "success"},
                {"message", "Screenshot captured successfully."},
                {"image_base64", "AAAAFAKEIMAGEDATA=="},
                {"width", 1512},
                {"height", 982}
            };
            return ToolResult{result.dump(), true};
        }
        return ToolResult{json{{"status", "success"}, {"message", "ok"}}.dump(), true};
    }
};

int check(bool condition, const std::string& label) {
    std::cout << (condition ? "[PASS] " : "[FAIL] ") << label << "\n";
    return condition ? 0 : 1;
}

} // namespace

int main() {
    FakeLLMClient llm({
        R"({"thought":"look at screen","action":{"type":"tool_call","tool":"capture_screenshot","args":"{}"}})",
        R"({"thought":"click the button","action":{"type":"tool_call","tool":"click","args":"{\"x\":100,\"y\":200}"}})",
        R"({"thought":"verify","action":{"type":"tool_call","tool":"capture_screenshot","args":"{\"filename\":\"verify.png\"}"}})",
        R"({"thought":"all good","action":{"type":"finish","result":"All GUI steps completed: success"}})"
    });
    FakeEnvironment env;

    VisionAgentLoop loop(&llm, &env, 10, LoopThreshold{}, nullptr);
    AgentResult result = loop.run({
        Message{"system", "system prompt", {}},
        Message{"user", "click the button on screen", {}}
    });

    int failed = 0;
    failed += check(result.success, "loop finished successfully");
    failed += check(result.finalAnswer.contains("success"), "final answer passes the keyword evaluator");

    int messagesWithImage = 0;
    for (const Message& message : result.messages) {
        if (!message.images.empty()) {
            ++messagesWithImage;
        }
    }
    failed += check(messagesWithImage == 1, "only the latest screenshot is kept in history (got "
                                            + std::to_string(messagesWithImage) + ")");

    bool base64InText = false;
    for (const Message& message : result.messages) {
        if (message.content.find("AAAAFAKEIMAGEDATA") != std::string::npos) {
            base64InText = true;
        }
    }
    failed += check(!base64InText, "base64 data never leaks into the text content");

    bool sizeHintPresent = false;
    for (const Message& message : result.messages) {
        if (message.content.find("1512x982") != std::string::npos) {
            sizeHintPresent = true;
        }
    }
    failed += check(sizeHintPresent, "the model is told the screenshot coordinate space");

    std::cout << (failed == 0 ? "\nALL VISION LOOP TESTS PASSED\n" : "\nSOME TESTS FAILED\n");
    return failed == 0 ? 0 : 1;
}
