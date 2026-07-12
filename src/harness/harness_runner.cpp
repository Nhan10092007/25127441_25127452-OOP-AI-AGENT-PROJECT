#include"harness_runner.h"
#include"nlohmann/json.hpp"
#include<fstream>
#include<stdexcept>
#include<sstream>
#include"tools/calculator_tool.h"
#include"tools/exec_tool.h"
#include"tools/web_tool.h"
#include"tools/file_tool.h"
#include"tools/memory_tool.h"

using json = nlohmann::json;

HarnessConfig HarnessRunner::readHarnessConfig(const std::string& configPath) const{
    std::ifstream file(configPath);
    if(!file.is_open()){
        throw std::runtime_error("Error: Can't open config.json");
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();

    file.close();

    json config = json::parse(content);
    
    HarnessConfig res = {
        .llmConfig = {
            .base_URL = config["llm"]["base_URL"],
            .model_name = config["llm"]["model_name"],
            .temperature = config["llm"]["temperature"],
            .num_predict = config["llm"]["num_predict"]
        },
        .envConfig = {
            .mode = config["environment"]["mode"],
            .workspace = config["environment"]["workspace"]
        },
        .thresholdConfig = {
            .repeatWarning = config["loop_threshold"]["repeat_warning"],
            .repeatCritical = config["loop_threshold"]["repeat_critical"],
            .pingpongWarning = config["loop_threshold"]["pingpong_warning"],
            .pingpongCritical = config["loop_threshold"]["pingpong_critical"]
        }
    };

    return res;
}

std::vector<Task> HarnessRunner::readTasks(const std::string& tasksPath) const{
    std::ifstream file(tasksPath);
    if(!file.is_open()){
        throw std::runtime_error("Error: Can't open tasks.json");
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();

    std::string content = buffer.str();
    json tasks = json::parse(content);
    std::vector<Task> res;
    for(const auto& task : tasks){
        Task temp;
        temp.id = task["id"];
        temp.description = task["description"];
        temp.instruction= task["instruction"];
        temp.eval_type = task["eval_type"];
        temp.eval_script_windows = task.value("eval_script_windows", "");
        temp.eval_script_linux = task.value("eval_script_linux", "");
        temp.max_steps = task["max_steps"];
        res.push_back(temp);
    }
    return res;
}

std::string HarnessRunner::toLower(const std::string& str) const{
    std::string result = str;
    for(char& c : result){
        c = std::tolower(static_cast<unsigned char>(c));
    }
    return result;
}

HarnessRunner::HarnessRunner(const std::string& configPath, const std::string& skillsPath, const std::string& tasksPath): 
    config(readHarnessConfig(configPath)),
    skillLoader(skillsPath),
    threshold(config.thresholdConfig)
{
    client = std::make_unique<OllamaClient>(config.llmConfig);
    
    toolRegistry = std::make_shared<ToolRegistry>();
    
    toolRegistry->registerTool<CalculatorTool>("calculator");
    toolRegistry->registerTool<ExecTool>("exec");
    toolRegistry->registerTool<ReadFileTool>("read_file");
    toolRegistry->registerTool<WriteFileTool>("write_file");
    toolRegistry->registerTool<MemorySave>("memory_save");
    toolRegistry->registerTool<MemorySearch>("memory_search");
    toolRegistry->registerTool<WebTool>("web_search");


    std::string mode = toLower(config.envConfig.mode);
    if(mode == "sandbox"){
        std::unique_ptr<NativeEnvironment> native = std::make_unique<NativeEnvironment>(toolRegistry);
        env = std::make_unique<SandboxEnvironment>(std::move(native), config.envConfig);
    }
    else if(mode == "native"){
        env = std::make_unique<NativeEnvironment>(toolRegistry);
    }
    else{
        throw std::runtime_error("Error: Invalid environment mode");
    }
    
    tasksList = readTasks(tasksPath);
}