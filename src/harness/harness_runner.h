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
    std::string eval_script;
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
    HarnessConfig readHarnessConfig(const std::string& configPath) const;
    std::vector<Task> readTasks(const std::string& tasksPath) const;
    std::string toLower(const std::string& str) const;
public:
    HarnessRunner(const std::string& configPath, const std::string& skillsPath, const std::string& tasksPath);
    
};