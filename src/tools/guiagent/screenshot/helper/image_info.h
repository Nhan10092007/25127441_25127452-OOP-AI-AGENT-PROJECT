#pragma once
#include <string>

// Đọc kích thước (pixel) của một file PNG bằng cách parse chunk IHDR.
// Trả về false nếu file không tồn tại hoặc không phải PNG hợp lệ.
bool readPngSize(const std::string& filePath, int& width, int& height);
