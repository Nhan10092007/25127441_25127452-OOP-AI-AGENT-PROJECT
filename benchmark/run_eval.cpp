#include <iostream>
#include <vector>
#include <curl/curl.h>
#include "client/ollama_client.h" // Nhớ include đúng đường dẫn nếu khác thư mục

int main() {
    // 1. Khởi tạo libcurl toàn cục (Mày có note lại vụ này ở cuối file ollama_client.cpp đó)
    curl_global_init(CURL_GLOBAL_ALL);

    try {
        // 2. Setup Config (Khởi tạo cứng để test lẹ, mốt thay bằng hàm loadConfig từ file .json sau)
        LLMConfig config = {
            .base_URL = "https://debug-animate-citizen.ngrok-free.dev/app/chat", // Nhớ thay link nếu ngrok bị reset
            .model_name = "gemma4:e4b",
            .temperature = 0.0f, // Để 0.0 để model trả lời chính xác, ko ảo giác
            .num_predict = 256
        };

        // Khởi tạo Client
        LLMClient *client = new OllamaClient(config);

        // 3. TẠO MOCK DATA ĐỂ TEST
        std::vector<Message> mock_messages = {
            {
                .role = "system",
                .content = "Bạn là một trợ lý AI hữu ích. Hãy trả lời ngắn gọn, trực diện và chính xác.",
                .images = {} // Trống vì mình đang test text trước
            },
            {
                .role = "user",
                .content = "Tính 1 + 1 bằng bao nhiêu?",
                .images = {}
            }
        };

        // 4. Gửi request và hứng kết quả
        std::cout << "Dang gui request den Ollama qua ngrok..." << std::endl;
        LLMResponse response = client->sendRequest(mock_messages);

        // 5. In kết quả ra console
        std::cout << "\n=== KET QUA TU OLLAMA ===" << std::endl;
        std::cout << "Phan hoi: " << response.response << std::endl;
        std::cout << "Prompt Tokens: " << response.prompt_token << std::endl;
        std::cout << "Answer Tokens: " << response.answer_token << std::endl;
        std::cout << "Total Tokens: " << response.total_token << std::endl;
        std::cout << "Trang thai success: " << (response.success ? "True" : "False") << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "\nBUG DETECTED: " << e.what() << std::endl;
    }

    // 6. Dọn dẹp libcurl
    curl_global_cleanup();

    return 0;
}