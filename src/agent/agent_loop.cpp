#include "agent_loop.h"
#include <iostream>
#include <chrono>
#include <thread>

AgentLoop::AgentLoop(LLMClient *client, Environment *env, int max_steps, const LoopThreshold &threshold, std::function<void(const StepRecord &)> hook) : llmClient(client), environment(env), max_steps(max_steps), loopDetector(threshold), hook(hook){}

void AgentLoop::observe()
{
    if (!hasNewObservation){
        return;
    }
    std::string observationText = "";

    if (lastToolResult.success){
        observationText = "Observation: " + lastToolResult.content;
    }
    else{
        observationText = "Observation Error: " + lastToolResult.content;
    }
    Message newMessage;
    newMessage.role = "user";
    newMessage.content = observationText;
    this->messages.push_back(newMessage);
    hasNewObservation = false;
    lastToolResult = ToolResult{};
}

LLMResponse AgentLoop::think() // Thêm logic retry nếu tunnels ngrok bị lỗi:
{
    const int MAX_RETRIES = 3;
    int attempts = 0;

    while (true) {
        try {
            return llmClient->sendRequest(this->messages);
        }
        catch (const std::exception& e) {
            ++attempts;
            std::cerr << "[AgentLoop] LLM request failed (attempt " << attempts << "/" << MAX_RETRIES << "): " << e.what() << "\n";

            if (attempts >= MAX_RETRIES) {
                throw std::runtime_error("Failed due to network error after " + std::to_string(MAX_RETRIES) +" attempts: " + e.what());
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }
}

ToolResult AgentLoop::act(const ToolInput &call)
{
    ToolResult result = environment->step(call);
    return result;
}

std::optional<Action> AgentLoop::parseAction(const std::string& rawText) {
    json data;
    try {
        data = json::parse(rawText);
    } catch (const json::parse_error&) {
        return std::nullopt; 
    }

    if (!data.contains("action") || !data["action"].is_object()) {
        return std::nullopt;
    }

    const json& actionObj = data["action"];
    std::string type = actionObj.value("type", "");

    if (type == "tool_call") {
        if (!actionObj.contains("tool") || !actionObj.contains("args")) {
            return std::nullopt; // thiếu field bắt buộc
        }
        std::string toolName = actionObj["tool"].get<std::string>();
        std::string args;
        if (actionObj["args"].is_string()) {
            args = actionObj["args"].get<std::string>();
        } else {
            args = actionObj["args"].dump();
        }
        return Action{ToolCallAction{toolName, args}};
    }
    else if (type == "finish") {
        std::string result = actionObj.value("result", "");
        return Action{FinishAction{result}};
    }
    else if (type == "error") {
        std::string message = actionObj.value("message", "");
        return Action{ErrorAction{message}};
    }

    return std::nullopt; 
}

AgentResult AgentLoop::run(std::vector<Message> initialMessages) {
    this->messages = initialMessages; 
    loopDetector.reset();
    current_step = 0;
    hasNewObservation = false;
    lastToolResult = ToolResult{};

    while (current_step < max_steps) {
        observe();

        LLMResponse llmResponse;
        int latency_ms = 0;
        try {
            auto start = std::chrono::steady_clock::now();
            llmResponse = think();
            auto end = std::chrono::steady_clock::now();
            latency_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        }
        catch (const std::exception& e) {
            if (hook) {
                StepRecord record;
                record.step_id = current_step;
                record.thought = "";
                record.action = json{{"type", "error"}, {"message", e.what()}};
                record.parse_error = std::string("Network error: ") + e.what();
                record.tokens_used = 0;
                record.answer_token = 0;
                record.prompt_token = 0;
                record.latency_ms = latency_ms;
                hook(record);
            }
            return AgentResult{false, e.what(), this->messages, current_step + 1};
        }

        this->messages.push_back(Message{"assistant", llmResponse.response, {}});
        std::string thought;
        try {
            json parsed = json::parse(llmResponse.response);
            thought = parsed.value("thought", "");
        } catch (const json::exception&) {
            thought = ""; 
        }
        std::optional<Action> actionOpt = parseAction(llmResponse.response);

        if (!actionOpt) {
            this->messages.push_back(Message{
                "user",
                "Your response was not valid JSON or did not match the required format. "
                "Please respond strictly following the JSON format in the system prompt. "
                "Do NOT include any text outside the JSON block.",
                {}
            });
            if (hook) {
                StepRecord record;
                record.step_id = current_step;
                record.thought = thought;
                record.action = json{{"type", "parse_error"}};
                record.parse_error = "LLM response is not valid JSON or missing required fields";
                record.tokens_used = llmResponse.total_token;
                record.answer_token = llmResponse.answer_token;
                record.prompt_token = llmResponse.prompt_token;
                record.latency_ms  = latency_ms;
                hook(record);
            }

            ++current_step;
            continue; 
        }
        bool shouldReturn = false;
        AgentResult returnValue;

        std::visit([&](auto&& actionVariant) {
            using T = std::decay_t<decltype(actionVariant)>;

            if constexpr (std::is_same_v<T, FinishAction>) {

                if (hook) {
                    StepRecord record;
                    record.step_id     = current_step;
                    record.thought     = thought;
                    record.action      = actionToJson(*actionOpt);
                    record.tokens_used = llmResponse.total_token;
                    record.answer_token = llmResponse.answer_token;
                    record.prompt_token = llmResponse.prompt_token;
                    record.latency_ms  = latency_ms;
                    hook(record);
                }

                returnValue  = AgentResult{true, actionVariant.finalAnswer, messages, current_step + 1};
                shouldReturn = true;
            }

            else if constexpr (std::is_same_v<T, ErrorAction>) {

                if (hook) {
                    StepRecord record;
                    record.step_id     = current_step;
                    record.thought     = thought;
                    record.action      = actionToJson(*actionOpt);
                    record.tokens_used = llmResponse.total_token;
                    record.answer_token = llmResponse.answer_token;
                    record.prompt_token = llmResponse.prompt_token;
                    record.latency_ms  = latency_ms;
                    hook(record);
                }

                returnValue  = AgentResult{false, actionVariant.message, messages, current_step + 1};
                shouldReturn = true;
            }

            else if constexpr (std::is_same_v<T, ToolCallAction>) {
                LoopStatus loopStatus = loopDetector.record(actionVariant.toolName, actionVariant.args);

                if (loopStatus == LoopStatus::CRITICAL) {

                    if (hook) {
                        StepRecord record;
                        record.step_id     = current_step;
                        record.thought     = thought;
                        record.action      = actionToJson(*actionOpt);
                        record.loop_warning = true;
                        record.tokens_used = llmResponse.total_token;
                        record.answer_token = llmResponse.answer_token;
                        record.prompt_token = llmResponse.prompt_token;
                        record.latency_ms  = latency_ms;
                        hook(record);
                    }

                    returnValue  = AgentResult{
                        false,
                        "Loop detected: " + actionVariant.toolName,
                        messages,
                        current_step + 1
                    };
                    shouldReturn = true;

                } else {
                    
                    ToolResult toolResult = this->act(ToolInput{actionVariant.toolName, actionVariant.args});
                    lastToolResult = toolResult;
                    hasNewObservation = true;

                    if (hook) {
                        StepRecord record;
                        record.step_id     = current_step;
                        record.thought     = thought;
                        record.action      = actionToJson(*actionOpt);
                        record.tool_result = toolResult.content;
                        record.success     = toolResult.success;
                        record.loop_warning = (loopStatus == LoopStatus::WARNING);
                        record.tokens_used = llmResponse.total_token;
                        record.answer_token = llmResponse.answer_token;
                        record.prompt_token = llmResponse.prompt_token;
                        record.latency_ms  = latency_ms;
                        hook(record);
                    }
                }
            }
        }, *actionOpt);

        if (shouldReturn) return returnValue;
        ++current_step;
    }

    return AgentResult{false, "Max steps reached", this->messages, max_steps};
}