#pragma once
#include <string>

namespace gui_utils {

// Kiểm tra một chương trình ngoài (xdotool, cliclick, ...) có tồn tại trong PATH hay không.
bool commandExists(const std::string& program);

// Ném runtime_error kèm hướng dẫn cài đặt nếu thiếu chương trình phụ thuộc.
// NativeEnvironment sẽ bắt exception này và trả thẳng thông báo về cho agent.
void requireCommand(const std::string& program, const std::string& installHint);

// Chờ một chút cho giao diện kịp cập nhật sau mỗi hành động GUI.
void waitForUi(int milliseconds = 400);

} // namespace gui_utils
