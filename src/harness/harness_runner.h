#pragma once

#include"tools/tool_registry.h"
#include"agent/skill_loader.h"
#include"agent/loop_detector.h"
#include"client/ollama_client.h"
#include<fstream>
#include"nlohmann/json.hpp"
#include<memory>
#include"env/sandbox_environment.h"
#include"env/native_environment.h"
#include<vector>
#include<string>

using json = nlohmann::json;

class HarnessRunner{
private:
    ToolRegistry toolRegistry;
    SkillLoader skillLoader;
    std::unique_ptr<LLMClient> client;
    std::unique_ptr<Environment> env;
    LoopThreshold threshold;
    std::vector<std::string> tasksList;

public:
    
};