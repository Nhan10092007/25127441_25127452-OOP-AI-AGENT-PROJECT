#include "vision_agent_loop.h"
#include <iostream>

using json = nlohmann::json;

// Chỉ giữ lại ảnh của lần quan sát mới nhất.
// Nếu để dồn mọi screenshot trong lịch sử hội thoại, context của VLM sẽ phình rất nhanh
// (mỗi ảnh vài trăm KB base64) và model dễ nhầm ảnh cũ với trạng thái màn hình hiện tại.
void VisionAgentLoop::dropPreviousImages() {
    for (Message& message : this->messages) {
        if (!message.images.empty()) {
            message.images.clear();
            message.content += " [Screenshot from this step was removed from history; only the latest screenshot is kept.]";
        }
    }
}

void VisionAgentLoop::observe() {
    if (!hasNewObservation) {
        return;
    }

    Message newMessage;
    newMessage.role = "user";
    try {
        json resultJson = json::parse(lastToolResult.content);
        if (resultJson.contains("image_base64")) {
            dropPreviousImages();

            std::string sizeHint;
            if (resultJson.contains("width") && resultJson.contains("height")) {
                sizeHint = " The image is " + std::to_string(resultJson["width"].get<int>()) + "x" +
                           std::to_string(resultJson["height"].get<int>()) +
                           " and its pixel coordinates are exactly the coordinates the 'click' tool expects.";
            }
            newMessage.content =
                "Observation: Screenshot captured successfully. Analyze the attached image to decide "
                "your next GUI action (click, type_text, or key_press)." + sizeHint;
            newMessage.images.push_back(resultJson["image_base64"].get<std::string>());

            std::cout << "  [VisionAgent] Injected screenshot into LLM's vision.\n";
        }
        else {
            newMessage.content = (lastToolResult.success ? "Observation: " : "Observation Error: ")
                                 + lastToolResult.content;
        }
    }
    catch (const json::exception&) {
        newMessage.content = (lastToolResult.success ? "Observation: " : "Observation Error: ")
                             + lastToolResult.content;
    }

    this->messages.push_back(newMessage);
    hasNewObservation = false;
    lastToolResult = ToolResult{};
}
