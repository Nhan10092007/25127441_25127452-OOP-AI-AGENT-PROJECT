#pragma once

#include "environment.h"
#include<filesystem>
#include<memory>

namespace fs = std::filesystem;

// Dùng Decorator Pattern giữa SandboxEnvironment và NativeEnvironmet
// Ta sẽ gọi hàm step() của NativeEnvironment bên trong SandboxEnvironmet (Sandbox bọc Native)
// Do step() của cả 2 giống nhau chỉ khác ở Sandbox có kiểm tra lỗi => step() của sandbox chỉ cần check lỗi, rồi dùng step() của Native luôn
// => Điều này tránh việc lặp code, ghi lại hàm step() y chang chỉ khác mỗi cái check lỗi

class SandboxEnvironment : public Environment{
private:
    std::unique_ptr<Environment> wrapped;  // Dùng con trỏ này để bọc lấy Native Environment
    fs::path workspaceRoot;

    // Helper Functions:
    std::optional<fs::path> isSafePath(const std::string& args);
    bool isSafeCommand(const std::string &args);
public:
    SandboxEnvironment(std::unique_ptr<Environment> inner, const EnvironmentConfig& config);
    ToolResult step(const ToolInput& toolRequest) override;
};