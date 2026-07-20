#pragma once

#include"tools/tool_registry.h"
#include"agent/skill_loader.h"
#include"agent/loop_detector.h"
#include"client/ollama_client.h"
#include<memory>
#include"env/sandbox_environment.h"
#include"env/native_environment.h"
#include<vector>
#include<string>
#include<filesystem>
#include<agent/agent_loop.h>

namespace fs = std::filesystem;

struct HarnessConfig{
    LLMConfig llmConfig;
    EnvironmentConfig envConfig;
    LoopThreshold thresholdConfig;
};

struct Task{
    std::string id;
    std::string description;
    std::string instruction;
    std::string eval_type;
    std::string eval_script_windows;
    std::string eval_script_linux;
    int max_steps;
};

class HarnessRunner{
private:
    HarnessConfig config;
    std::shared_ptr<ToolRegistry> toolRegistry;
    SkillLoader skillLoader;
    std::unique_ptr<LLMClient> client;
    std::unique_ptr<Environment> env;
    LoopThreshold threshold;
    std::vector<Task> tasksList;
    
    // Helper function:
    HarnessConfig readHarnessConfig(const fs::path& configPath) const;
    std::vector<Task> readTasks(const fs::path& tasksPath) const;
    std::string toLower(const std::string& str) const;
    void archiveWorkspace(const std::string& taskId);
    double calcSuccessRate(int passCount, int numberOfTasks) const;
    void exportTaskReport(const Task& task, const AgentResult& result, bool isPass, const fs::path& reportRoot);
    void exportBatchSummary(int passCount, double successRate, int totalTasks, const fs::path& reportRoot);
public:
    HarnessRunner(const fs::path& configPath, const fs::path& skillsPath, const fs::path& tasksPath);
    void runBatch();
};