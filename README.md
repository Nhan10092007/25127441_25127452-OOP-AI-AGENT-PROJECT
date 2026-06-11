# AI AGENT WITH OLLAMA API
# Đồ án môn học: Lập trình hướng đối tượng (OOP)

## Thông tin thành viên nhóm:
- **Lưu Thiện Nhân - 25127441**
- **Châu Tiến Phát - 25127452**

# A. Hướng dẫn Setup Ollama API với model Gemma4 trên Google Colab:
## Update soon





# B. Hướng dẫn cài đặt và biên dịch:

## Hướng dẫn cài đặt CMake:
## Update soon



## Cài đặt các thư viện c++ cần thiết cho dự án:
- **nlohmann/json**: Đã được cài sẵn trong thư mục `include/` (file `json.hpp`) (Header-only) nên thầy không cần cài gì thêm.
- **SQLite3**: Đã được tích hợp sẵn dưới dạng mã nguồn Amalgamation trong thư mục `include/` (file `sqlite3.h`) và `src/` (file `sqlite3.c`) nên thầy không cần cài gì thêm.
- **libcurl**: Với thư viện này ta cần cài đặt trên hệ thống trước khi build.

### **Hướng dẫn cài đặt thư viện libcurl:

### 1. Đối với người dùng hệ điều hành Windows:
- **Bước 1**: Mở thanh tìm kiếm Windows, bật công cụ **MSYS2 UCRT64** (hoặc dùng Terminal UCRT64 đã được tích hợp sẵn trong Visual Studio Code).
- **Bước 2**: Nhập lệnh sau để tự động tải và đồng bộ thư viện CURL chuẩn UCRT 64-bit:
```bash
pacman -S mingw-w64-ucrt-x86_64-curl
```
- **Bước 3**: Sau khi cài đặt hoàn tất, ta nên khởi động lại Visual Studio Code để hệ thống nạp lại biến môi trường mới.

### 2. Đối với người dùng hệ điều hành Linux (Ubuntu / Debian):
- **Bước 1**: Mở Terminal và cập nhật danh sách các gói hệ thống:
```bash
sudo apt-get update
```
- **Bước 2**: Cài đặt gói `libcurl` phiên bản phát triển (bao gồm đầy đủ file thư viện `.so` và các file tiêu đề `.h` cần thiết cho quá trình biên dịch):
```bash
sudo apt-get install libcurl4-openssl-dev
```
### 3. Đối với người dùng macOS (Homebrew):
- **Bước 1**: Mở Terminal trên Mac và chạy lệnh cài đặt:
```bash
brew install curl
```
- **Bước 2**: ***(Lưu ý)*** Thường macOS đã có sẵn một bản curl cũ của hệ thống. Nếu CMake không tự nhận diện được bản mới vừa cài, hãy chạy lệnh sau để ép hệ thống ưu tiên dùng bản của Homebrew:
```bash
echo 'export PATH="/usr/local/opt/curl/bin:$PATH"' >> ~/.zshrc
source ~/.zshrc
```

## Hướng dẫn build và biên dịch bằng command line:

### Nâng cấp phiên bản trình biên dịch để có thể biên dịch C++26:

Đầu tiên, ta cần kiểm tra phiên bản trình biên dịch trên hệ thống bằng lệnh `g++ --version` hoặc `clang++ --version`.
Do dự án này sử dụng tiêu chuẩn `C++26`, nó yêu cầu **`g++` tối thiểu là phiên bản 14** hoặc **`clang++` tối thiểu là phiên bản 18**. Nếu phiên bản hiện tại thấp hơn, chúng ta phải tiến hành cập nhật theo từng hệ điều hành tương ứng như sau:

### 1. Đối với người dùng Windows (MSYS2 UCRT64):
Ta sẽ cài đặt trong MSYS2 UCRT64 của Windows bằng các lệnh sau
- **Bước 1**: Cập nhật toàn bộ các gói phần mềm và MSYS2 lên bản mới nhất
```bash
pacman -Syu
```
- **Bước 2**: Tải và cài đặt bộ trình biên dịch GCC chuẩn UCRT 64-bit
```bash
pacman -S mingw-w64-ucrt-x86_64-gcc
```

### 2. Đối với người dùng Linux (Ubuntu / Debian):
- **Bước 1**: Thêm một kho chứa phần mềm bên thứ 3 (PPA) vào hệ thống
```bash
sudo add-apt-repository ppa:ubuntu-toolchain-r/test
```
- **Bước 2**: Báo cho hệ thống biết là ta vừa thêm vào kho chứa phần mềm mới
```bash
sudo apt update
```
- **Bước 3**: Tải phiên bản `g++-14` về máy
```bash
sudo apt install g++-14
```
- **Bước 4**: Nếu CMake vẫn nhận bản `g++` cũ, hãy gõ lệnh xuất biến môi trường
```bash
export CXX=g++-14
```

### 3. Đối với người dùng macOS (Homebrew):
Mặc định macOS sử dụng Apple Clang. Để biên dịch C++26 mượt mà nhất, ta cần cài đặt bản LLVM Clang gốc qua Homebrew:
- **Bước 1**: Làm mới danh bạ của Homebrew
```bash
brew update
```
- **Bước 2**: Tải bộ trình biên dịch LLVM (chứa clang++ phiên bản mới nhất)
```bash
brew install llvm
```
- **Bước 3**: Ép CMake sử dụng clang++ của LLVM vừa cài thay vì Apple Clang mặc định của máy
```bash
export CXX=$(brew --prefix llvm)/bin/clang++
```

### Cấu hình IntelliSense trong Visual Studio Code để loại bỏ lỗi gạch đỏ do compiler:
- **Bước 1**: Trong giao diện Visual Studio Code, ấn tổ hợp phím `Ctrl + Shift + p` (Windows/Linux) hoặc `Cmd + Shift + p` (macOS).
- **Bước 2**: Ấn `C/C++: Edit Configurations (UI)` sau đó `Enter`.
- **Bước 3**: Kéo xuống đến phần `C++ standard`, ta chỉnh thành `c++26`.
- **Bước 4** (Bước kiểm tra lại): Khi chỉnh thành `c++26`, lúc này trong project của chúng ta sẽ xuất hiện thư mục `.vscode/` và trong thư mục ấy sẽ có file `c_cpp_properties.json` ta ấn vào file đó. Nếu ta thấy có mục `"cppStandard": "c++26"` thì ta đã hoàn thành.

### Do các cấu hình xây dựng đã được định nghĩa trong file `CMakeLists.txt`, chúng ta có thể tiến hành biên dịch theo các bước sau:

# Phần này để tạm, sẽ sửa sau

- **Bước 1**: Khởi tạo thư mục `build` và nạp cấu hình CMake (Chỉ cần chạy 1 lần đầu tiên hoặc khi có thay đổi trong file `CMakeLists.txt`):
  - **Trên Windows (MSYS2):**
    ```bash
    cmake -G "MinGW Makefiles" -B build
    ```
  - **Trên Linux / macOS:**
    ```bash
    cmake -B build
    ```

- **Bước 2**: Tiến hành biên dịch (build) và chạy chương trình (Sử dụng lệnh này cho mọi lần chạy sau khi sửa code):
  - **Trên Windows:**
    ```bash
    cmake --build build && ./build/agent_runner.exe
    ```
  - **Trên Linux / macOS:**
    ```bash
    cmake --build build && ./build/agent_runner
    ```