#include"native_environment.h"
#include<stdexcept>

NativeEnvironment::NativeEnvironment(std::shared_ptr<ToolRegistry> registry):
    toolRegistry(registry)
{}

ToolResult NativeEnvironment::step(const ToolInput& toolRequest){
    std::unique_ptr<Tool> tool = toolRegistry->createTool(toolRequest.toolName);
    
    if(!tool){ // Nếu không có tool đó tồn tại thì trả về nullptr
        return ToolResult{"Error: Can't found tool " + toolRequest.toolName, false};
    }

    try{
        std::string result = tool->execute(toolRequest.args);
        return ToolResult{result, true};
    }
    catch(const std::exception& e){
        return ToolResult{"Error " + toolRequest.toolName + " tool: " + e.what(), false};
    }
}