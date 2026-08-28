#include "exec_tool.h"
#include <string>
#include <array>    // Thêm thư viện array để làm bộ đệm (buffer)
#include <stdexcept>
#ifdef _WIN32
#include<windows.h>
#else
#include<unistd.h>
#include<sys/wait.h>
#include<signal.h>
#include<fcntl.h>
#endif
#include<chrono>
#include<thread>
#include<vector>

ExecTool::ExecTool() : Tool("exec", 
    #ifdef _WIN32
        "Tool for executing system command (shell command). You are currently on Windows. Args parameter: A string containing the shell command (in Windows, you aren't allowed to use Linux/macOS commands). (Example: 'dir')."
    #else
        "Tool for executing system command (shell command). You are currently on Linux/macOS. Args parameter: A string containing the shell command (in Linux/macOS, you aren't allowed to use Windows commands). (Example: 'ls -la')."
    #endif
){}

std::string ExecTool::execute(const std::string& args) {
    if (args.empty()) {
        throw std::runtime_error("Missing args");
    }
    std::string command = args;
    std::string result = "";

    // DWORD : unsigned int 32 bit
    // HANDLE : Tiến trình
    // pid_t : integer

    #ifdef _WIN32
        command = "cmd.exe /c " + args; // Thêm tiền tố cmd.exe /c để nó tìm đến file cmd.exe để thực thi

        SECURITY_ATTRIBUTES sa; // Tạo cấu trúc định nghĩa bảo mật cho pipe
        sa.nLength = sizeof(SECURITY_ATTRIBUTES); // Set kích thước của cấu trúc
        sa.bInheritHandle = TRUE; // Cho phép các HANDLE tạo ra từ cấu trúc này được thừa kế bởi các tiến trình con
        sa.lpSecurityDescriptor = NULL; // Sử dụng bảo mật mặc định 

        HANDLE hReadPipe;
        HANDLE hWritePipe;
        CreatePipe(&hReadPipe, &hWritePipe, &sa, 0); // Hàm tạo pipe gồm đầu đọc, ghi

        // Đảm bảo đầu đọc không kế thừa (chỉ cần đầu ghi kế thừa cho tiến trình con)
        SetHandleInformation(hReadPipe, HANDLE_FLAG_INHERIT, 0);

        STARTUPINFOA si = {0};
        si.cb = sizeof(STARTUPINFOA);
        si.dwFlags |= STARTF_USESTDHANDLES;
        si.hStdOutput = hWritePipe;
        si.hStdError = hWritePipe; // Gộp cả stderr vào chung

        PROCESS_INFORMATION pi = {0};

        std::vector<char> cmdBuffer(command.begin(), command.end());
        cmdBuffer.push_back('\0');

        BOOL success = CreateProcessA(
            NULL,                  // Không chỉ định application name riêng, dùng luôn command line
            cmdBuffer.data(),      // Command line (phải mutable buffer, không phải string literal)
            NULL, NULL,
            TRUE,                  // bInheritHandles = TRUE, để tiến trình con nhận được hWritePipe
            CREATE_NO_WINDOW,      // Không hiện cửa sổ console phụ
            NULL, NULL,
            &si, &pi
        );

        if(!success){
            throw std::runtime_error("Failed to create process!");
        }

        CloseHandle(hWritePipe); 

        const DWORD TIMEOUT_MS = 10000; // 10s
        DWORD waitResult = WaitForSingleObject(pi.hProcess, TIMEOUT_MS); 

        if(waitResult == WAIT_TIMEOUT){
            TerminateProcess(pi.hProcess, 1);   // Buộc kill tiến trình con
            WaitForSingleObject(pi.hProcess, INFINITE); // Đợi nó thực sự chết hẳn (dọn dẹp)
            result = "Command timed out after " + std::to_string(TIMEOUT_MS/1000) + " seconds. It may be waiting for interactive input or running indefinitely. Try a different, non-interactive command.";
        }
        else{
            std::array<char, 4096> buffer;
            DWORD bytesRead;
            while(ReadFile(hReadPipe, buffer.data(), buffer.size() - 1, &bytesRead, NULL) && bytesRead > 0){ // Đọc dữ liệu và truyền từng phần vào buffer
                buffer[bytesRead] = '\0';
                result += buffer.data();
            }

            // Phân biệt việc hết dữ liệu với việc đọc file lỗi
            DWORD lastError = GetLastError();
            if(lastError != ERROR_BROKEN_PIPE && lastError != ERROR_SUCCESS){
                CloseHandle(hReadPipe);
                CloseHandle(pi.hProcess);
                CloseHandle(pi.hThread);
                throw std::runtime_error("ReadFile failed with error code: " + std::to_string(lastError));
            }

            // Lấy exit code để biết có thực hiện thành công hay ko
            // exitCode == 0 => Thành công, exitCode != 0 => Thất bại
            DWORD exitCode = 0;
            GetExitCodeProcess(pi.hProcess, &exitCode);
            if(exitCode != 0){
                result = "[Exit code: " + std::to_string(exitCode) + "]\n" + result;
            }

        }
        CloseHandle(hReadPipe);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);

    #else
        int pipefd[2];
        if(pipe(pipefd) == -1){ // pipefd[0] = đầu đọc, pipefd[1] = đầu ghi
            throw std::runtime_error("Failed to create pipe!");
        }

        pid_t pid = fork();

        if(pid == -1){
            throw std::runtime_error("Failed to fork process!");
        }

        if(pid == 0){
            // Tiến trình con
            close(pipefd[0]);              // Con không cần đầu đọc
            dup2(pipefd[1], STDOUT_FILENO); // Trỏ stdout của con vào pipe
            dup2(pipefd[1], STDERR_FILENO); // Gộp luôn stderr
            close(pipefd[1]);
            execl("/bin/sh", "sh", "-c", command.c_str(), (char*)NULL);
            _exit(127); // Nếu execl thất bại (lệnh không tồn tại)
        }

        // Đóng tiến trình cha
        close(pipefd[1]);

        // Pipe của HĐH chỉ chứa được khoảng 64KB. Nếu cha đợi con chết xong mới đọc,
        // lệnh nào in ra nhiều hơn 64KB sẽ làm con bị chặn khi ghi => deadlock, và cha
        // báo nhầm là "timed out". Vì vậy phải vừa đợi vừa đọc, với đầu đọc non-blocking.
        int flags = fcntl(pipefd[0], F_GETFL, 0);
        fcntl(pipefd[0], F_SETFL, flags | O_NONBLOCK);

        auto start = std::chrono::steady_clock::now();
        const int TIMEOUT_SEC = 10;
        int status = 0;
        pid_t result_pid;

        bool isTimeOut = false;
        bool childExited = false;
        std::array<char, 4096> buffer;

        auto drainPipe = [&]() {
            ssize_t bytesRead;
            while ((bytesRead = read(pipefd[0], buffer.data(), buffer.size() - 1)) > 0) {
                buffer[bytesRead] = '\0';
                result += buffer.data();
            }
        };

        while (true) {
            drainPipe(); // Đọc trước để tiến trình con không bị nghẽn khi ghi

            result_pid = waitpid(pid, &status, WNOHANG); // Không chặn, chỉ hỏi "xong chưa"
            if (result_pid == pid) {
                childExited = true;
                drainPipe(); // Vét nốt phần còn lại trong pipe
                break;
            }

            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - start).count();
            if (elapsed >= TIMEOUT_SEC) {
                kill(pid, SIGKILL);       // Buộc kill
                waitpid(pid, &status, 0); // Đợi dọn dẹp
                result = "Command timed out after " + std::to_string(TIMEOUT_SEC) + " seconds. It may be waiting for interactive input or running indefinitely. Try a different, non-interactive command.";
                isTimeOut = true;
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(50)); // Nghỉ ngắn rồi hỏi lại
        }

        if(!isTimeOut && childExited){
            // Lấy exit code
            if(WIFEXITED(status)){
                int exitCode = WEXITSTATUS(status);
                if(exitCode != 0){
                    result = "[Exit code: " + std::to_string(exitCode) + "]\n" + result;
                }
            }
        }
        close(pipefd[0]);

    #endif

    if(result.empty()){
        return "Executed successfully, but no output.";
    }

    // Cắt bớt output quá dài: num_ctx của model chỉ vài nghìn token, một lệnh in ra hàng MB
    // sẽ làm tràn context và phá hỏng toàn bộ hội thoại của agent.
    const std::size_t MAX_OUTPUT = 8000;
    if(result.size() > MAX_OUTPUT){
        std::string head = result.substr(0, 5000);
        std::string tail = result.substr(result.size() - 2000);
        result = head
               + "\n\n[... " + std::to_string(result.size() - 7000)
               + " characters omitted because the output was too long ...]\n\n"
               + tail;
    }

    return result;
}