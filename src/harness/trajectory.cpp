#include"trajectory.h"
#include<fstream>
#include<stdexcept>

using json = nlohmann::json;

Trajectory::Trajectory(const std::string& taskId, const std::string& AImodel): task_id(taskId), model(AImodel) {}

void Trajectory::addStep(const StepRecord& newStep){
    steps.push_back(newStep);
}

void Trajectory::setSuccess(bool evaluatorResult){
    success = evaluatorResult;
}

int Trajectory::calcTotalTokens() const{
    int totalTokens = 0;
    for(const auto& step : steps){
        totalTokens += step.tokens_used;
    }
    return totalTokens;
}

int Trajectory::calcTotalTime() const{
    int totalTime = 0;
    for(const auto& step : steps){
        totalTime += step.latency_ms;
    }
    return totalTime;
}

void Trajectory::exportToJson(const fs::path& filePath){
    if(!fs::exists(filePath) || !fs::is_directory(filePath)){
        throw fs::filesystem_error("File system error: The trajectory folder is not exist", fs::path(filePath), std::make_error_code(std::errc::no_such_file_or_directory));
    }
    fs::path completePath = filePath / ("trajectory" + task_id + ".json");
    std::ofstream file(completePath);
    if(!file.is_open()){
        throw std::runtime_error("Can't open " + completePath.string());
    }
    try{
        json trajectory;
        trajectory["task_id"] = task_id;
        trajectory["model"] = model;
        trajectory["success"] = success;
        trajectory["total_tokens"] = calcTotalTokens();
        trajectory["total_time_ms"] = calcTotalTime();

        json jsonSteps = json::array();
        for(const auto& step : steps){
            json jsonStep;
            jsonStep["step_id"] = step.step_id;
            jsonStep["thought"] = step.thought;
            jsonStep["action"] = step.action;
            
            if(step.tool_result != std::nullopt){
                jsonStep["tool_result"] = step.tool_result.value();
            }
            if(step.success != std::nullopt){
                jsonStep["success"] = step.success.value();
            }
            if(step.loop_warning != std::nullopt){
                jsonStep["loop_warning"] = step.loop_warning.value();
            }
            if(step.parse_error != std::nullopt){
                jsonStep["parse_error"] = step.parse_error.value();
            }

            jsonStep["tokens_used"] = step.tokens_used;
            jsonStep["answer_tokens"] = step.answer_token;
            jsonStep["prompt_tokens"] = step.prompt_token;
            jsonStep["latency_ms"] = step.latency_ms;

            jsonSteps.push_back(jsonStep);
        }
        trajectory["steps"] = jsonSteps;

        file << trajectory.dump(4);
        file.close();
    }
    catch(const std::exception& e){
        throw std::runtime_error(std::string("Error: ") + e.what()); // std::string operator+(const char*, const std::string&);
    }
}