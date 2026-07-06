#include"skill_loader.h"
#include<fstream>
#include<sstream>
#include<stdexcept>


// Thuật toán Trim:
std::string SkillLoader::trim(const std::string &str) const{
    auto first = str.find_first_not_of(" \t\r\n");
    if(first == std::string::npos){
        return "";
    }
    auto last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

std::vector<std::string> SkillLoader::splitKeywords(const std::string& s) const{
    std::vector<std::string> result;
    std::stringstream ss(s);
    std::string item;
    while(std::getline(ss, item, ',')){
        std::string keyword = trim(item);
        if(!keyword.empty()){
            result.push_back(keyword);
        }
    }
    return result;
}

SkillLoader::~SkillLoader() = default;

SkillLoader::SkillLoader(const fs::path& skillsFolder){
    if(!fs::is_directory(skillsFolder)){
        throw fs::filesystem_error("File system error: The directory skills is not exist", fs::path(skillsFolder), std::make_error_code(std::errc::no_such_file_or_directory));
    }
    for(const auto& entry : fs::directory_iterator(skillsFolder)){
        if(entry.is_regular_file() && entry.path().extension() == ".md"){ // Xem xem file đó có phải file bth không và nó có phải md không
            std::ifstream file(entry.path());
            if(!file.is_open()){
                throw std::runtime_error("Can't open file: " + entry.path().filename().string());
            }
            std::vector<std::string> keywords;
            std::string content = "";
            std::string line;

            bool inFrontmatter = false;
            bool frontmatterDone = false;

            while(std::getline(file, line)){
                if(!frontmatterDone && line.find("---") == 0){
                    if(!inFrontmatter){
                        inFrontmatter = true;
                    }
                    else{
                        inFrontmatter = false;
                        frontmatterDone = true;
                    }
                    continue;
                }

                if(inFrontmatter){
                    if(line.find("keywords:") == 0){
                        std::string kwString = line.substr(9); // Cắt phần "keyword:", lấy từ index 9 trở đi.
                        keywords = splitKeywords(kwString);
                    }
                }
                else{
                    content += line + "\n";
                }
            }
            file.close();

            std::string filename = entry.path().stem().string();

            if(filename == "task_planner"){
                _taskPlanner = trim(content);
            }
            else if(filename == "error_recovery"){
                _errorRecovery = trim(content);
            }
            else{
                skillKeywords[filename] = keywords;
                skillStorage[filename] = trim(content);
            }

        }
    }
    if(_taskPlanner.empty() || _errorRecovery.empty()){
        throw std::runtime_error("Error: Lack task_planner.md or error_recovery.md in folder skills!");
    }
}

std::string SkillLoader::getSkills(const std::vector<std::string>& skillsName) const{
    for(const std::string& skill : skillsName){
        if(!skillStorage.contains(skill)){
            throw std::runtime_error("Error: Can't find skill " + skill + " in skillStorage!");
        }
        else if(skillStorage.at(skill).empty()){
            throw std::runtime_error("Error: " + skill + " skill's file is empty!");
        }
    }
    std::string res;
    for(const std::string& skill : skillsName){ 
        res += skillStorage.at(skill) + "\n\n";
    }
    return _taskPlanner + "\n\n" + _errorRecovery + "\n\n" + res;
}

const std::unordered_map<std::string, std::vector<std::string>>& SkillLoader::getSkillKeywords() const{
    return skillKeywords;
}

std::string SkillLoader::toLower(const std::string& str) const{
    std::string result = str;
    for(char& c : result){
        c = std::tolower(static_cast<unsigned char>(c));
    }
    return result;
}

std::vector<std::string> SkillLoader::selectSkills(const std::string& prompt) const{
    if(prompt.empty()){
        throw std::runtime_error("Empty prompt: Can't select skills");
    }
    std::string lowerPrompt = toLower(prompt);
    std::vector<std::string> matchSKills;
    for(const auto& [skill, keywords] : skillKeywords){
        for(const std::string& keyword : keywords){
            if(lowerPrompt.find(toLower(keyword)) != std::string::npos){
                matchSKills.push_back(skill);
                break;
            }
        }
    }
    return matchSKills;
}