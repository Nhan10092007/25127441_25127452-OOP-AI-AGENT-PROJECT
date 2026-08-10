# AI AGENT WITH OLLAMA API
# Đồ án môn học: Lập trình hướng đối tượng (OOP)

### Thông tin thành viên nhóm:
- **Lưu Thiện Nhân - 25127441**
- **Châu Tiến Phát - 25127452**

# A. Hướng dẫn Setup Ollama API với model Gemma4 trên Google Colab:
## Hướng dẫn thứ tự chạy các Cell trong Google Colab:
**Đường dẫn đến Google Colab mà nhóm em đã chuẩn bị: [Google Colab link](https://colab.research.google.com/drive/1n_PJcvaE19ps4LB-QGNtnnffFAa4_Pb0?usp=sharing)**<br>
Do chúng em đã setup hết trên Google Colab nên thầy chỉ việc chạy các cell theo thứ tự sau:
- **Kết nối GPU của Google Colab**: Khi vào giao diện của Google Colab trong đường link mà em đã gửi, ở góc phải bên trên, ta sẽ thấy có chữ `Kết nối`. Thầy phải ấn vô đó để nó kết nối với GPU. (Phải có GPU hỗ trợ nếu không sẽ không chạy được model)
- **Cell 1**: Chạy script setup môi trường và cài các thư viện python cần thiết
```bash
!sudo apt update
!apt-get install -y zstd
!sudo apt install -y pciutils
!curl -fsSL https://ollama.com/install.sh | sh
!pip install fastapi uvicorn pydantic httpx pyngrok nest-asyncio
```
- **Cell 2**: Cài đặt môi trường và kết nối với Ollama sever
```python
import os
env = os.environ.copy()
env["OLLAMA_HOST"] = "0.0.0.0" 
env["OLLAMA_ORIGINS"] = "*" 
import subprocess
def run_ollama_serve():
  subprocess.Popen(["ollama", "serve"], env=env)
import threading
thread = threading.Thread(target = run_ollama_serve)
thread.start()
import time
time.sleep(5)
```
- **Cell 3**: Kéo modle Gemm4 về từ Ollama
```bash
!ollama pull gemma4:e4b
```
- **Cell 4**: Thiết lập endpoint và điều chỉnh dữ liệu trả về từ response của Ollama bằng `fastAPI`
```python
from fastapi import FastAPI
import uvicorn
from pydantic import BaseModel
import httpx
import threading
from typing import List, Dict, Any
class DataInput(BaseModel):
  model: str
  messages: List[Dict[str, Any]]
  stream: bool = False
  options: Dict[str, Any]
app = FastAPI()
ollama_url = "http://127.0.0.1:11434/api/chat"
@app.post("/app/chat")
async def chat_endpoint(payload: DataInput):
  valid_data = payload.model_dump()
  try:
    async with httpx.AsyncClient(timeout = 120.0) as client:
      response = await client.post(ollama_url, json=valid_data)
      response.raise_for_status()
      ollama_response = response.json()
      filter_response = {
          "response" : ollama_response.get("message", {}).get("content",""),
          "prompt_token" : ollama_response.get("prompt_eval_count", 0),
          "answer_token" : ollama_response.get("eval_count", 0),
          "total_token" : ollama_response.get("prompt_eval_count", 0) + ollama_response.get("eval_count", 0),
          "success" : ollama_response.get("done", False),
      }
      return filter_response
  except Exception as e:
    return {"error" : str(e) }
def run_uvicorn_sever():
  uvicorn.run(app, host = "0.0.0.0", port = 8000)
thread = threading.Thread(target = run_uvicorn_sever, daemon = True)
thread.start()
print("Uvicorn sever run on http://0.0.0.0:8000")
```
- **Cell 5**: Thiết lập Tunnels để kết nối với code c++ bằng `ngrok` (**Lưu ý: Thầy cần xem phần `Hướng dẫn lấy ngrok Authtoken` ở dưới trước khi chạy Cell này**)
```python
from pyngrok import ngrok
ngrok.kill()
ngrok.set_auth_token("DÁN MÃ NGROK CÁ NHÂN CỦA THẦY VÀO ĐÂY")
port = 8000
public_url = ngrok.connect(port).public_url
print("Ngrok tunnels connecting from local c++ code to Uvicorn sever: ", public_url)
```
Sau khi chạy cell này xong, nó sẽ hiện ra dòng:
```bash
Ngrok tunnels connecting from local c++ code to Uvicorn sever:  https://debug-animate-citizen.ngrok-free.dev
```
Thầy hãy copy đường link `https://debug-animate-citizen.ngrok-free.dev` và ấn cell tiếp theo.

- **Cell 6**: Chạy Warm-up để tải model lên VRAM:
```python
import httpx
import time
api_url = "http://127.0.0.1:8000/app/chat"
payload = {
    "model": "gemma4:e4b",
    "messages": [
        {"role": "user", "content": "hi"}
    ],
    "stream": False,
    "options": {
        "num_predict": 1
    }
}
start_time = time.time()
try:
    response = httpx.post(api_url, json=payload, timeout=180.0)
    response.raise_for_status()

    data = response.json()
    end_time = time.time()

    print(f"Warm-up finish sucessfully in {end_time - start_time:.2f} seconds!")
    print(f"response: {data}")
except Exception as e:
    print(f"Error during warm-up: {type(e).__name__}: {str(e)}")
```

**SAU KHI ẤN XONG CELL 6 THÌ COI NHƯ TA ĐÃ HOÀN TẤT THIẾT LẬP OLLAMA API VÀ CHUYỂN SANG BƯỚC TIẾP THEO `Hướng dẫn cấu hình model trong dự án`**

- **Cell 7**: Tắt Tunnels hiện tại của `ngrok` (**Lưu ý: Thầy chỉ chạy Cell này khi đã test xong**)
```python
ngrok.kill()
```

## Hướng dẫn lấy ngrok Authtoken:
- **Bước 1**: Truy cập: [ngrok.com](https://ngrok.com/) và thực hiện đăng kí tài khoản.
- **Bước 2**: Khi đăng kí xong, ta vào phần `Setup & Installation`, ở mục `Your Authtoken`, ta ấn nút copy để lấy Authtoken.
- **Bước 3**: Dán Authtoken vào **cell 5** ở dòng:
```python
ngrok.set_auth_token("DÁN MÃ NGROK CÁ NHÂN CỦA THẦY VÀO ĐÂY")
```
Thầy hãy thay chữ `DÁN MÃ NGROK CÁ NHÂN CỦA THẦY VÀO ĐÂY` bằng Authtoken của thầy.

## Hướng dẫn cấu hình model trong dự án:
- **Bước 1**: Trong thư mục dự án, thầy sẽ thấy có một thư mục `config/` và trong đó sẽ có file `config.example.json`. Thầy hãy đổi tên file ấy thành `config.json`.
- **Bước 2**: Trong file `config.json` ta sẽ thấy:
```python
{
  "llm": {
    "base_URL": "DÁN ĐƯỜNG LINK NGROK CỦA THẦY VÀO ĐÂY/app/chat",
    "model_name": "gemma4:e4b",
    "temperature": 0.3,
    "num_predict": 2048,
    "num_ctx": 8192
  },
  "environment": {
    "mode": "sandbox",
    "workspace": "workspace/"
  },
  "loop_threshold": {
    "repeat_warning": 2,
    "repeat_critical": 3,
    "pingpong_warning": 2,
    "pingpong_critical": 3
  }
}
```
+ Các field thầy nên giữ nguyên: `model_name` (model mặc định theo gợi ý đồ án), `num_ctx`,  `num_predict`, `mode` (Ta nên ưu tiên test bằng chế độ sandbox thay vì native), `workspace` (thư mục cho các hoạt động của write_file và read_file tool trong sandbox mode).
+ `base_URL`: Thầy sẽ dán đường link Ngrok mà nãy thầy đã copy trong Google Colab vào phần `DÁN ĐƯỜNG LINK NGROK CỦA THẦY VÀO ĐÂY` (Lưu ý: Thầy phải giữ nguyên endpoint `/app/chat`).
+ `temperature`: Thầy nên giữ chỉ số này ở khoảng 0.0 - 0.3.
+ `loop_threshold`: Đây là cấu hình cho `LoopDetector`. Trong đó, `pingpong_warning` và `repeat_warning` được dùng để kích hoạt cảnh báo sớm. Nếu số lần lặp chạm ngưỡng tối đa là `pingpong_critical` hoặc `repeat_critical`, tiến trình của task sẽ bị hủy ngay lập tức.


# B. Hướng dẫn cài đặt và biên dịch:

## Hướng dẫn cài đặt CMake:
Do dụ án yêu cầu Cmake bản **3.30** trở lên để có thể biên dịch nên cách nhanh nhất là cài qua package manager của hệ điều hành (Cách này tự động lấy bản ổn định mới nhất và tự thêm vào PATH, không cần cấu hình thêm). <br>
Có thể chạy các lệnh dưới đây bằng bất kỳ terminal nào (CMD, PowerShell, Terminal app, hoặc Terminal tích hợp trong VSCode đều như nhau).

### 1. Đối với người dùng hệ điều hành Windows:
```bash
winget install Kitware.CMake
```
Sau khi cài xong, thầy nên đóng và mở lại terminal mới để PATH được nạp lại.
### 2. Đối với người dùng hệ điều hành macOS (Homebrew):
```bash
brew install cmake
```
### 3. Đối với người dùng hệ điều hành Linux (Ubuntu/Debian):
```bash
sudo apt update
sudo apt install cmake
```
### Cách kiểm tra xem đã cài thành công hay chưa:
```bash
cmake --version
```
Kết quả hiển thị sẽ có version từ phiên bản 3.30 trở lên.

- **Lưu ý:** Sau khi cài, nếu terminal đang mở không nhận lệnh `cmake`, hãy đóng và mở lại terminal mới (kể cả terminal trong VSCode) — PATH chỉ được nạp lại khi mở session mới.

- **Phương án dự phòng:** Nếu máy không hỗ trợ các lệnh trên (ví dụ Windows không có `winget`, hoặc macOS chưa cài Homebrew), có thể tải installer trực tiếp tại [cmake.org/download](https://cmake.org/download/) — chọn bản nằm dưới mục `stable release` mới nhất, tránh các bản có hậu tố `-rc` (release candidate, chưa ổn định).


## Cài đặt các thư viện c++ cần thiết cho dự án:
- **nlohmann/json**: Đã được cài sẵn trong thư mục `include/` (file `json.hpp`) (Header-only) nên thầy không cần cài gì thêm.
- **SQLite3**: Đã được tích hợp sẵn dưới dạng mã nguồn Amalgamation trong thư mục `include/` (file `sqlite3.h`) và `src/` (file `sqlite3.c`) nên thầy không cần cài gì thêm.
- **libcurl**: Với thư viện này ta cần cài đặt trên hệ thống trước khi build.

## Hướng dẫn cài đặt thư viện libcurl:

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

# Hướng dẫn build và biên dịch bằng command line:

## Nâng cấp phiên bản trình biên dịch để có thể biên dịch C++26:

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

## Cấu hình IntelliSense trong Visual Studio Code để loại bỏ lỗi gạch đỏ do compiler:
- **Bước 1**: Trong giao diện Visual Studio Code, ấn tổ hợp phím `Ctrl + Shift + p` (Windows/Linux) hoặc `Cmd + Shift + p` (macOS).
- **Bước 2**: Ấn `C/C++: Edit Configurations (UI)` sau đó `Enter`.
- **Bước 3**: Kéo xuống đến phần `C++ standard`, ta chỉnh thành `c++26`.
- **Bước 4** (Bước kiểm tra lại): Khi chỉnh thành `c++26`, lúc này trong project của chúng ta sẽ xuất hiện thư mục `.vscode/` và trong thư mục ấy sẽ có file `c_cpp_properties.json` ta ấn vào file đó. Nếu ta thấy có mục `"cppStandard": "c++26"` thì ta đã hoàn thành.

## Do các cấu hình xây dựng đã được định nghĩa trong file `CMakeLists.txt`, chúng ta có thể tiến hành biên dịch trên terminal theo các bước sau:

- **Bước 1**: Khởi tạo thư mục `build` và nạp cấu hình CMake (Chỉ cần chạy 1 lần đầu tiên hoặc khi có thay đổi trong file `CMakeLists.txt`):
  - **Đối với người dùng Windows (MSYS2):**
    ```bash
    cmake -G "MinGW Makefiles" -B build
    ```
  - **Đối với người dùng Linux / macOS:**
    ```bash
    cmake -B build
    ```

- **Bước 2**: Tiến hành biên dịch (build) và chạy chương trình (Sử dụng lệnh này cho mọi lần chạy sau khi sửa code):
  - **Đối với người dùng Windows:**
    ```bash
    cmake --build build ; ./build/agent_runner.exe
    ```
  - **Đối với người dùng Linux / macOS:**
    ```bash
    cmake --build build && ./build/agent_runner
    ```

# Xem kết quả sau khi chạy chương trình:

Sau khi chạy xong, nếu thầy thấy kết quả trên terminal hiện như sau:
```bash
START RUN BATCH...
task_001 FINISH!
task_002 FINISH!
task_003 FINISH!
task_004 FINISH!
task_005 FINISH!
task_006 FINISH!
task_007 FINISH!
task_008 FINISH!
task_009 FINISH!
task_010 FINISH!
FINISH RUNNING BATCH!
```
Nó có nghĩa là tất cả các tasks đã được hoành thành xong và thầy có thể xem kết quả ở các thư mục sau:
1. `trajectory/`: Đây là nơi ghi lại chi tiết các quyết định, suy nghĩ và hành động của agent từ đầu đến cuối. Nó cho ta biết được các thông tin như: task đấy có thành công hay không, token mà task đó sử dụng là bao nhiêu, thời gian thực thi task, suy nghĩ và hành động của AI khi giải quyết tasks,.... Giúp ta có cái nhìn tổng quát nhất về quy trình thực hiện của agent với vòng lặp ReAct.
2. `archive/`: Là nơi lưu trữ các directories/files mà AI tạo ra theo yêu cầu của task (được chuyển qua từ `workspace/`). Với các tasks yêu cầu AI ghi kết quả ra file thì ta có thể kiểm tra trực tiếp các file kết quả ấy trong thư mục này để xem agent có thực hiện đúng hay chưa.
3. `report/`: Đây là nơi lưu trữ các tệp `messages.json` ghi lại conversation history của từng task. Điều này giúp ta có thể dễ dàng xem được những lỗi mà ta gặp phải là gì và AI đã suy nghĩ như thế nào khi nhận task và giải quyết. Ngoài ra trong thư mục này còn lưu trữ các file `result.txt` ghi lại các thông tin như: Thực thi thành công hay không, final awnser của agent dành cho task đó, số step cần để giải quyết bằng cách đó và nhận xét về khả năng thực thi của agent dựa trên số step (Hoàn thành với mức step thấp -> Hiệu quả cao).