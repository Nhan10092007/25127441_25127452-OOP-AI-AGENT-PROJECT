#pragma once

#include<string>
#include<unordered_map>
#include<filesystem>
#include<vector>

// Kĩ thuật c++17: filesystem
namespace fs = std::filesystem;

class SkillLoader{
private:
    std::string _taskPlanner;
    std::string _errorRecovery;
    std::unordered_map<std::string, std::string> skillStorage;
public:
    SkillLoader(const fs::path skillsFolder);
    ~SkillLoader();
    std::string getSkills(const std::vector<std::string>& skillsName);
};