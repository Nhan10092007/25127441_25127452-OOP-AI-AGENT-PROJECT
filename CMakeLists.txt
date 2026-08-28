# Phiên bản Cmake tối thiểu cho dự án (c++26)
cmake_minimum_required(VERSION 3.30)

# Đặt tên cho project
project(AI_Agent_Project)

# Ép xài chuẩn c++ 26
set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Lệnh này giúp folder include trở thàng một cái gốc nên ta chỉ cần include"nlohmann/json.hpp". Không cần thêm thư mục include ở đầu
include_directories(${CMAKE_SOURCE_DIR}/include)

# Tượng tự trên do các folder/file trong thư mục src tương tác qua lại rất nhiều nên ta để src làm gốc và ta ko cần phải cho src vào include nữa
include_directories(${CMAKE_SOURCE_DIR}/src)

# Cho cmake tự đi tìm libcurl có sẵn trong máy
find_package(CURL REQUIRED)

# Đóng gói các file lại để dễ quản lí và chạy
set(CLIENT_SOURCE 
src/client/ollama_client.cpp
src/client/embed_client.cpp
)
set(ENVIRONMENT_SOURCE
src/env/native_environment.cpp
src/env/sandbox_environment.cpp
)
set(AGENT_SOURCE 
src/agent/agent_loop.cpp
src/agent/vision_agent_loop.cpp
src/agent/loop_detector.cpp
src/agent/skill_loader.cpp
)
set(TOOL_SOURCE 
src/tools/calculator/calculator_tool.cpp
src/tools/exec/exec_tool.cpp
src/tools/file/file_tool.cpp
src/tools/memory/memory_tool.cpp
src/tools/web/web_tool.cpp
src/tools/listdir/list_dir_tool.cpp
src/tools/datetime/datetime_tool.cpp
src/tools/stringtool/string_tool.cpp
src/tools/tool_policy.cpp
src/tools/tool_registry.cpp
src/tools/guiagent/screenshot/screenshot_tool.cpp
src/tools/guiagent/screenshot/helper/screenshot_capture.cpp
src/tools/guiagent/screenshot/helper/encodeBase64.cpp
src/tools/guiagent/mouse_click/mouse_click_tool.cpp
src/tools/guiagent/mouse_click/helper/mouse_executor.cpp
src/tools/guiagent/keyboard_type/type_press_tool.cpp
src/tools/guiagent/keyboard_type/key_press_tool.cpp
src/tools/guiagent/keyboard_type/helper/keyboard_executor.cpp
)
set(HARNESS_SOURCE
src/harness/functional_evaluator.cpp
src/harness/keyword_evaluator.cpp
src/harness/trajectory.cpp
src/harness/harness_runner.cpp
)

# Đúc thành file agent_runner.exe để chạy
add_executable(agent_runner benchmark/run_eval.cpp src/sqlite3.c ${AGENT_SOURCE} ${ENVIRONMENT_SOURCE} ${CLIENT_SOURCE} ${TOOL_SOURCE} ${HARNESS_SOURCE})

# Lắp thư viện libcurl vào file để chạy
target_link_libraries(agent_runner PRIVATE CURL::libcurl)
