#include"skill_loader.h"
#include<fstream>
#include<sstream>
#include<stdexcept>

SkillLoader::~SkillLoader() = default;

SkillLoader::SkillLoader(const fs::path skillsFolder){
    if(!fs::is_directory(skillsFolder)){
        throw fs::filesystem_error("File system error: The directory skills is not exist", fs::path(skillsFolder), std::make_error_code(std::errc::no_such_file_or_directory));
    }
    for(const auto& entry : fs::directory_iterator(skillsFolder)){
        if(entry.is_regular_file() && entry.path().extension() == ".md"){ // Xem xem file đó có phải file bth không và nó có phải md không
            std::ifstream file(entry.path());
            if(!file.is_open()){
                throw std::runtime_error("Can't open file: " + entry.path().filename().string());
            }
            else{
                std::stringstream buffer;
                buffer << file.rdbuf(); // .rdbuf() trả về con trỏ trỏ thẳng vào luồng raw bytes => Lấy tất cả nội dung cho vào buffer dưới dạng bytes
                std::string content = buffer.str();

                if(entry.path().filename().string() == "task_planner.md"){
                    _taskPlanner = content;
                }
                else if(entry.path().filename().string() == "error_recovery.md"){
                    _errorRecovery = content;
                }
                else{
                    skillStorage[entry.path().stem().string()] = content; //.stem chỉ lưu tên file, không lưu loại file
                }
            }
            file.close();
        }
    }
    if(_taskPlanner.empty() || _errorRecovery.empty()){
        throw std::runtime_error("Error: Lack task_planner.md or error_recovery.md in folder skills!");
    }
}

std::string SkillLoader::getSkills(const std::vector<std::string>& skillsName){
    for(const std::string& skill : skillsName){
        if(!skillStorage.contains(skill)){
            throw std::runtime_error("Error: Can't find skill " + skill + " in skillStorage!");
        }
        else if(skillStorage[skill].empty()){
            throw std::runtime_error("Error: " + skill + " skill's file is empty!");
        }
    }
    std::string res;
    for(const std::string& skill : skillsName){ 
        res += skillStorage[skill] + "\n\n";
    }
    return _taskPlanner + "\n\n" + _errorRecovery + "\n\n" + res;
}