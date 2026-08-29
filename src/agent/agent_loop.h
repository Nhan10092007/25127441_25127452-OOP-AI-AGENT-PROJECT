#pragma once
#include <string>
#include <vector>
#include <functional>
#include "tools/tool_registry.h"
#include "client/llm_client.h"
#include "env/environment.h"
#include "loop_detector.h"
#include "steprecord.h"
#include "action.h"

struct AgentResult {
    bool success;
    std::string finalAnswer;
    std::vector<Message> messages;   
    int stepsTaken;
};

class AgentLoop {
protected:
    LLMClient* llmClient;    
    Environment* environment;  
    LoopDetector loopDetector; 
    std::vector<Message> messages; 
    int max_steps;
    int current_step;
    std::function<void(const StepRecord&)> hook;
    ToolResult lastToolResult;      
    bool hasNewObservation = false; 
public:
    AgentLoop(LLMClient* client, Environment* env, int max_steps, const LoopThreshold& threshold, std::function<void(const StepRecord&)> hook);
    virtual ~AgentLoop() = default;   // Cần virtual vì lớp con (VisionAgentLoop) được xoá qua con trỏ AgentLoop*
    virtual AgentResult run(std::vector<Message> initialMessages);

protected:
    virtual void observe();
    virtual LLMResponse think();
    virtual std::optional<Action> parseAction(const std::string& rawText);
    virtual ToolResult act(const ToolInput& call);
};