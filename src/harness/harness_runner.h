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
#include"client/embed_client.h"

namespace fs = std::filesystem;

struct HarnessConfig{
    LLMConfig llmConfig;
    EmbeddingConfig embeddingConfig;
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
    std::unique_ptr<EmbeddingClient> embeddingClient;
    LoopThreshold threshold;
    std::vector<Task> tasksList;
    fs::path trajectoriesPath;
    fs::path reportsPath;
    fs::path archivesPath;


    // Helper function:
    HarnessConfig readHarnessConfig(const fs::path& configPath) const;
    std::vector<Task> readTasks(const fs::path& tasksPath) const;
    std::string toLower(const std::string& str) const;
    void archiveWorkspace(const std::string& taskId);
    double calcSuccessRate(int passCount, int numberOfTasks) const;
    void exportTaskReport(const Task& task, const AgentResult& result, bool isPass, const fs::path& reportRoot);
    void exportBatchSummary(int passCount, double successRate, int totalTasks, const fs::path& reportRoot);
public:
    HarnessRunner(const fs::path& configPath, const fs::path& skillsPath, const fs::path& tasksPath, const fs::path& trajectoryPath, const fs::path& reportPath, const fs::path& archivePath);
    void runBatch();
};