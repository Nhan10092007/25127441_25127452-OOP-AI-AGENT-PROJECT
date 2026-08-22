#include"harness_runner.h"
#include"nlohmann/json.hpp"
#include<fstream>
#include<stdexcept>
#include<sstream>
#include"client/llm_client.h"
#include"tools/calculator/calculator_tool.h"
#include"tools/exec/exec_tool.h"
#include"tools/web/web_tool.h"
#include"tools/file/file_tool.h"
#include"tools/memory/memory_tool.h"
#include "tools/guiagent/screenshot/screenshot_tool.h"
#include "tools/guiagent/mouse_click/mouse_click_tool.h"
#include "tools/guiagent/keyboard_type/type_press_tool.h"
#include "tools/tool_policy.h"
#include "agent/vision_agent_loop.h"
#include<iostream>
#include"trajectory.h"
#include"keyword_evaluator.h"
#include"functional_evaluator.h"
#include<functional>

using json = nlohmann::json;

HarnessConfig HarnessRunner::readHarnessConfig(const fs::path& configPath) const{
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
            .num_predict = config["llm"]["num_predict"],
            .num_ctx = config["llm"]["num_ctx"]
        },
        .embeddingConfig = {
            .model_name = config["embedding"]["model_name"],
            .base_URL = config["embedding"]["base_URL"],
            .similarity_threshold = config["embedding"]["similarity_threshold"]
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

std::vector<Task> HarnessRunner::readTasks(const fs::path& tasksPath) const{
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

void HarnessRunner::archiveWorkspace(const std::string& taskId){
    try{
        fs::path newArchivePath = fs::path(archivesPath) / (taskId);
        fs::create_directories(newArchivePath);
        for(const auto& entry : fs::directory_iterator(config.envConfig.workspace)){
            fs::rename(entry, newArchivePath / entry.path().filename()); // Move all files from ./workspace to ./archive/TaskXX/ to store result
        }
        if(fs::exists("memory.db")){
            fs::rename("memory.db", newArchivePath / "memory.db"); // Mỗi task sẽ có một database riêng, không xài chung nữa
        }
    }
    catch(const std::exception& e){
        std::cerr<<"Error: " << e.what() << "\n";
    }
}

HarnessRunner::HarnessRunner(const fs::path& configPath, const fs::path& skillsPath, const fs::path& tasksPath, const fs::path& trajectoryPath, const fs::path& reportPath, const fs::path& archivePath):
    config(readHarnessConfig(configPath)),
    skillLoader(skillsPath),
    threshold(config.thresholdConfig),
    trajectoriesPath(trajectoryPath),
    reportsPath(reportPath),
    archivesPath(archivePath)
{
    client = std::make_unique<OllamaClient>(config.llmConfig);
    embeddingClient = std::make_unique<EmbeddingClient>(config.embeddingConfig);

    toolRegistry = std::make_shared<ToolRegistry>();
    
    toolRegistry->registerTool<ScreenshotTool>("capture_screenshot");
    toolRegistry->registerTool<MouseClickTool>("click");
    toolRegistry->registerTool<KeyboardTypeTool>("type_text");
    toolRegistry->registerTool<CalculatorTool>("calculator");
    toolRegistry->registerTool<ExecTool>("exec");
    toolRegistry->registerTool<ReadFileTool>("read_file");
    toolRegistry->registerTool<WriteFileTool>("write_file");
    toolRegistry->registerTool<WebTool>("web_search");
    toolRegistry->registerToolFactory("memory_save", [this]() -> std::unique_ptr<Tool> {
        return std::make_unique<MemorySave>(embeddingClient.get());
    });
    toolRegistry->registerToolFactory("memory_search", [this]() -> std::unique_ptr<Tool> {
        return std::make_unique<MemorySearch>(embeddingClient.get(), config.embeddingConfig.similarity_threshold);
    });


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
    try{
        fs::create_directories(config.envConfig.workspace);
        fs::create_directories(reportsPath);
        fs::create_directories(trajectoriesPath);
        fs::create_directories(archivesPath);
    }
    catch(const std::exception& e){
        throw std::runtime_error(e.what());
    }
}

double HarnessRunner::calcSuccessRate(int passCount, int numberOfTasks) const{
    return ((double)passCount / numberOfTasks) * 100;
}

void HarnessRunner::exportTaskReport(const Task& task, const AgentResult& result, bool isPass, const fs::path& reportRoot){
    try{
        fs::path taskDir = reportRoot / task.id;
        fs::create_directories(taskDir);
        if(!fs::exists(taskDir)){
            throw std::runtime_error("Task directory doesn't exits.");
        }

        json messages = json::array();
        for(const auto& [role, content, images] : result.messages){
            json temp;
            temp["role"] = role;
            temp["content"] = content;
            if(!images.empty()){
                temp["images"] = images;
            }
            messages.push_back(temp);
        }

        std::ofstream file;

        file.open(taskDir / ("messages.json"));
        if(!file.is_open()){
            throw std::runtime_error("Can't open messages.json.");
        }
        file << messages.dump(4);
        file.close();

        file.open(taskDir / ("result.txt"));
        if(!file.is_open()){
            throw std::runtime_error("Can't open result.txt.");
        }
        if(isPass){
            file << "- Result: Pass.\n";
        }
        else{
            file << "- Result: Fail.\n";
        }
        file << "- Final answer: " << result.finalAnswer << ".\n";
        file << "- Steps takens: " << result.stepsTaken << ".\n";
        
        if(isPass){
            double stepsRate = ((double)result.stepsTaken / task.max_steps) * 100;
            if(stepsRate < 30){
                file << "- Evaluate: Excellent performance. The agent solved this task quickly and efficiently, using only a small fraction of the allowed steps.";
            }
            else if(stepsRate <= 80){
                file << "- Evaluate: Solid performance. The agent completed the task successfully within a reasonable number of steps.";
            }
            else{
                file << "- Evaluate: The agent completed the task, but only after using most of the allowed steps. Consider reviewing the relevant skill or prompt to improve efficiency.";
            }
        }
        else{        
            if(result.stepsTaken >= task.max_steps){
                file << "- Evaluate: Agent failed to complete this task within the step limit — it may be stuck in an inefficient loop or the task is too difficult with current tools/skills.";
            }
            else{
                file << "- Evaluate: Agent stopped before reaching the step limit (finished or gave up early), but the answer was incorrect — the issue is likely with reasoning accuracy rather than running out of steps.";
            }
        }

        file.close();
    }
    catch(const std::exception& e){
        std::cerr<<"Error: "<<e.what()<<"\n";
    }
}

void HarnessRunner::exportBatchSummary(int passCount, double successRate, int totalTasks,const fs::path& reportRoot){
    try{
        fs::path summaryDirectory = reportRoot / "summary";
        fs::create_directories(summaryDirectory);
        if(!fs::exists(summaryDirectory)){
            throw std::runtime_error("Summary directory doesn't exist.");
        }
        json summary;
        summary["total_tasks"] = totalTasks;
        summary["passed"] = passCount;
        summary["failed"] = totalTasks - passCount;
        summary["success_rate"] = successRate;

        std::ofstream file;

        file.open(summaryDirectory / "summary.json");
        if(!file.is_open()){
            throw std::runtime_error("Can't open summary.json.");
        }
        file << summary.dump(4);
        file.close();

        file.open(summaryDirectory / "summary.txt");
        if(!file.is_open()){
            throw std::runtime_error("Can't open summary.txt.");
        }
        file << "- Total tasks: " << totalTasks << "\n";
        file << "- Passed: " << passCount << " / Failed: " << totalTasks - passCount << "\n";
        file << "- Success rate: " << successRate << "%\n";
        if(successRate >= 80){
            file << "- Evaluate: Excellent overall performance across the benchmark.";
        }
        else if(successRate >= 50){
            file << "- Evaluate: Moderate performance — review failed tasks to identify common issues.";
        }
        else{
            file << "- Evaluate: Poor overall performance — the agent's tool usage, skills, or prompting strategy likely need significant improvement.";
        }
        file.close();
    }
    catch(const std::exception& e){
        std::cerr<<"Error: "<<e.what()<<"\n";
    }
}

// Lambda function:
// [capture_clause](parameters) mutable -> return_type {
//     Body
// }

// capture_clause: Khai báo các biến từ môi trường bên ngoài mà lambda được phép sử dụng.
// []	Không truy cập bất kỳ biến nào bên ngoài.
// [=]	Bắt tất cả các biến bên ngoài bằng giá trị (Copy - chỉ đọc, không sửa được).
// [&]	Bắt tất cả các biến bên ngoài bằng tham chiếu (Reference - đọc và sửa được).
// [x, &y]	Bắt biến x bằng giá trị, và biến y bằng tham chiếu.
// [x] Bắt biến x bằng giá trị

// muatble: giúp bản sao ([=] || [x]) capture_clause mình đưa vào được thay đổi nhưng chỉ thay đổi bên trong hàm, ngoài hàm giữ nguyên

// smartpointer .get(): lấy địa chỉ của vùng nhớ mà smartpointer đang trỏ tới, trả về địa chỉ đó cho một raw pointer khác trỏ vào.
// Hết scope thì lấy lại địa chỉ ấy

void HarnessRunner::runBatch(){
    int passCount = 0;
    std::cout<<"START RUN BATCH...\n";
    for(const auto& task : tasksList){
        try{
            std::vector<std::string> necessarySkills = skillLoader.selectSkills(task.instruction);
            std::string systemPrompt = toolRegistry->getToolsDescription() +  skillLoader.getSkills(necessarySkills);
            Trajectory trajectory(task.id, config.llmConfig.model_name);
            AgentLoop loop(client.get(), env.get(), task.max_steps, threshold, [&trajectory](const StepRecord& record){
                trajectory.addStep(record);
            });
            Message system = {
                .role = "system",
                .content = systemPrompt
            };
            Message prompt = {
                .role = "user",
                .content = task.instruction
            };
            AgentResult result = loop.run({system, prompt});

            std::unique_ptr<Evaluator> evaluator;
            if(task.eval_type == "keyword"){
                evaluator = std::make_unique<KeywordEvaluator>();
            }
            else if(task.eval_type == "functional"){
                evaluator = std::make_unique<FunctionalEvaluator>();
            }
            else{
                throw std::runtime_error("Invalid evaluator type");
            }
            #ifdef _WIN32
            bool isPass = evaluator->evaluate(result.finalAnswer, task.eval_script_windows);
            #else
            bool isPass = evaluator->evaluate(result.finalAnswer, task.eval_script_linux);
            #endif

            exportTaskReport(task, result, isPass, reportsPath);

            trajectory.setSuccess(isPass);
            trajectory.exportToJson(trajectoriesPath);

            if(isPass){
                ++passCount;
            }
            
            archiveWorkspace(task.id);

            std::cout<<task.id<<" FINISH!\n";
        }
        catch(const std::exception& e){
            std::cerr << "Error: " << e.what() << "\n";
            std::cout << task.id << " FINISH DUE TO ERROR!\n";
        }
    }
    double successRate = calcSuccessRate(passCount, tasksList.size());
    exportBatchSummary(passCount, successRate, tasksList.size(), reportsPath);
    std::cout<<"FINISH RUNNING BATCH!\n";
}