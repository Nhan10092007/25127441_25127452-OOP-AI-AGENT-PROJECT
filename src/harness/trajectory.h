#pragma once

#include"steprecord.h"
#include<vector>
#include<filesystem>

namespace fs = std::filesystem;

class Trajectory{
private:
    std::string task_id;
    std::string model;
    bool success;
    std::vector<StepRecord> steps;

    // Helper functions:
    int calcTotalTokens() const;
    int calcTotalTime() const;

public:
    Trajectory(const std::string& taskId, const std::string& AImodel);
public:
    void addStep(const StepRecord& newStep);
    void setSuccess(bool evaluatorResult);
    void exportToJson(const fs::path& filePath);
};