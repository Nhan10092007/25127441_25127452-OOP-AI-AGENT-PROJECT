#include "vision_agent_loop.h"
#include <iostream>

using json = nlohmann::json;

void VisionAgentLoop::observe() {
    if (!hasNewObservation) {
        return; 
    }

    Message newMessage;
    newMessage.role = "user"; 
    try {
        json resultJson = json::parse(lastToolResult.content);
        if (resultJson.contains("image_base64")) {
            newMessage.content = "Observation: Screenshot captured successfully. Please analyze the image to decide your next GUI action (click, type_text, or key_press).";
            newMessage.images.push_back(resultJson["image_base64"].get<std::string>());
            
            std::cout << "  [VisionAgent] Injected screenshot into LLM's vision.\n";
        } 
        else {
            newMessage.content = "Observation: " + lastToolResult.content;
        }
    } 
    catch (...) {
        if (lastToolResult.success) {
            newMessage.content = "Observation: " + lastToolResult.content;
        } else {
            newMessage.content = "Observation Error: " + lastToolResult.content;
        }
    }

    this->messages.push_back(newMessage);
    hasNewObservation = false;
    lastToolResult = ToolResult{}; 
}
