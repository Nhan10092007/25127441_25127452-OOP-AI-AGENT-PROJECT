#include"sandbox_environment.h" 
#include"native_environment.h"
#include"nlohmann/json.hpp"
#include<optional>
#include<stdexcept>
#include<vector>

using json = nlohmann::json;

SandboxEnvironment::SandboxEnvironment(std::unique_ptr<Environment> inner, const EnvironmentConfig& config):
    wrapped(std::move(inner)),
    workspaceRoot(fs::weakly_canonical(config.workspace))
{}

std::optional<fs::path> SandboxEnvironment::isSafePath(const std::string& userPath){
    fs::path newFilePath = fs::weakly_canonical(workspaceRoot / userPath);
    fs::path rel = fs::relative(newFilePath, workspaceRoot);
    if(rel.empty() || rel.string().substr(0, 2) == ".."){ // Nếu sau khi chuẩn hóa mà có ../ thì có nghĩa là nó không nằm trong workspace
        return std::nullopt;
    }
    return newFilePath;
}
// Với trường hợp rel == "." thì nó rất hiếm và chỉ xảy ra khi userPath rỗng,... Với edge case này nó không quá nghiêm trọng và có thể được xử lí khi thực hiện tool.

std::string SandboxEnvironment::toLower(const std::string& str){ // Chuẩn hóa hết Command về lowercase
    std::string result = str;
    for(char& c : result){
        c = std::tolower(static_cast<unsigned char>(c));
    }
    return result;
}

// Static => cái này chỉ xài được trong sandbox_environment.cpp
static const std::vector<std::string> blackList  = {
    // For linux/macOS
    "rm -rf", "sudo", "mkfs", "dd ", "shutdown", "reboot", "halt",
    "poweroff", "chmod 777", "chown", "curl ", "wget ", ":(){ ",
    // For windows
    "format ", "del /f", "rd /s", "diskpart", "vssadmin delete shadows",
    "reg delete", "takeown ", "icacls ", "bcdedit", "cipher /w"
};

bool SandboxEnvironment::isSafeCommand(const std::string& userCommand){
    if(userCommand == ""){
        return false;
    }
    std::string lowerCommand = toLower(userCommand);
    for(const std::string& command : blackList){
        if(lowerCommand.find(command) != std::string::npos){
            return false;
        }
    }
    return true;
}

ToolResult SandboxEnvironment::step(const ToolInput& toolRequest){
    if(toolRequest.toolName == "write_file"){ // Kiểm tra đường dẫn
        try{
            json toolArgs = json::parse(toolRequest.args);
            std::string userPath = toolArgs["filename"];
            std::optional<fs::path> temp = isSafePath(userPath);            
            if(temp == std::nullopt){
                return ToolResult{"Invalid file path for write_file tool", false};
            }
            fs::path newFilePath = temp.value();
            ToolInput newRequest;
            newRequest.toolName = toolRequest.toolName;
            toolArgs["filename"] = newFilePath.string();
            newRequest.args = toolArgs.dump();

            return wrapped->step(newRequest);
        }
        catch(const std::exception& e){
            return ToolResult{"Error for write_file tool: " + std::string(e.what()), false};
        }
    }
    else if(toolRequest.toolName == "read_file"){ // Kiểm tra đường dẫn
        try{
            std::optional<fs::path> temp = isSafePath(toolRequest.args);
            if(temp == std::nullopt){
                return ToolResult{"Invalid file path for read_file tool", false};
            }
            fs::path newFilePath = temp.value();
            ToolInput newRequest;
            newRequest.toolName = toolRequest.toolName;
            newRequest.args = newFilePath.string();
        
            return wrapped->step(newRequest);
        }
        catch(const std::exception& e){
            return ToolResult{"Error for read_file tool: " + std::string(e.what()), false};
        }
    }
    else if(toolRequest.toolName == "capture_screenshot"){ // Ép ảnh chụp màn hình nằm trong workspace
        try{
            std::string userPath = "current_screen.png";
            if(!toolRequest.args.empty() && json::accept(toolRequest.args)){
                json toolArgs = json::parse(toolRequest.args);
                if(toolArgs.contains("filename") && toolArgs["filename"].is_string()){
                    userPath = toolArgs["filename"].get<std::string>();
                }
            }
            std::optional<fs::path> temp = isSafePath(userPath);
            if(temp == std::nullopt){
                return ToolResult{"Invalid file path for capture_screenshot tool", false};
            }
            json newArgs;
            newArgs["filename"] = temp.value().string();

            ToolInput newRequest;
            newRequest.toolName = toolRequest.toolName;
            newRequest.args = newArgs.dump();

            return wrapped->step(newRequest);
        }
        catch(const std::exception& e){
            return ToolResult{"Error for capture_screenshot tool: " + std::string(e.what()), false};
        }
    }
    else if(toolRequest.toolName == "exec"){ // Kiểm tra command
        try{
            if(!isSafeCommand(toolRequest.args)){
                return ToolResult{"Don't accept this command for exec tool. It's on the blacklist", false};
            }
        } 
        catch(const std::exception& e){
            return ToolResult{"Error for exec tool: " + std::string(e.what()), false};
        }
        // exec dùng chung return ở dưới với các file khác   
    }
    return wrapped->step(toolRequest);
}
