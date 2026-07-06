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
    std::unordered_map<std::string, std::vector<std::string>> skillKeywords;

    // Helper function:
    std::string trim(const std::string& str) const;
    std::vector<std::string> splitKeywords(const std::string& s) const;
    std::string toLower(const std::string& str) const;
public:
    SkillLoader(const fs::path& skillsFolder);
    ~SkillLoader();
    std::string getSkills(const std::vector<std::string>& skillsName) const;
    const std::unordered_map<std::string, std::vector<std::string>>& getSkillKeywords() const;
    std::vector<std::string> selectSkills(const std::string& prompt) const;
};