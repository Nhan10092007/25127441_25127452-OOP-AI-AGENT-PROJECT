#include"ollama_client.h"
#include"nlohmann/json.hpp"
#include<curl/curl.h>
#include<stdexcept>

using json = nlohmann::json;

OllamaClient::OllamaClient(const LLMConfig& config) : 
    _baseURL(config.base_URL), 
    _modelName(config.model_name), 
    _temperature(config.temperature),
    _numPredict(config.num_predict)
{};

OllamaClient::~OllamaClient() = default;

// size_t là kiểu dữ liệu unsigned integer dùng để biểu diễn số bytes của một object trong bộ nhớ
// Ta dùng size_t là bởi nó có thể thay đổi số bit dựa trên máy tính và phần mềm => Tránh tràn giá trị
// void* là con trỏ vô kiểu: nghĩa là chưa biết trỏ tới kiểu dữ liệu gì.
size_t callBack(void* contents, size_t size, size_t nbemb, void* userptr){
    size_t sumbyte = size * nbemb;
    // Ép về kiểu string* để trỏ vào tham chiếu của biến responseData ở dưới
    std::string* container = static_cast<std::string*>(userptr);
    char* trueData = static_cast<char*>(contents);
    container->append(trueData, sumbyte);   // Bốc chính xác sumbyte ký tự từ trueData cho vào container
    return sumbyte;
}

LLMResponse OllamaClient::sendRequest(const std::vector<Message> &messages){
    json json_payload;
    json_payload["model"] = _modelName;
    json_payload["stream"] = false;
    json_payload["options"] = {
        {"temperature", _temperature},
        {"num_predict", _numPredict}
    };
    json json_messages = json::array();
    // Structutred bindings:
    for(const auto& [role, content, images] : messages){
        json temp;
        temp["role"] = role;
        temp["content"] = content;
        if(!images.empty()){
            temp["images"] = images;
        }
        json_messages.push_back(temp);
    }
    json_payload["messages"] = json_messages;

    CURL* curl = curl_easy_init(); // Khởi tạo Handle quản lý phiên kết nối HTTP
    curl_slist* header = nullptr; // Con trỏ để gán nhãn
    header = curl_slist_append(
        header,
        "ngrok-skip-browser-warning: true" // Bỏ qua giai đoạn chặn của ngrok khi gửi request
    );
    header = curl_slist_append(
        header,
        "Content-Type: application/json" // Content-Type để báo cho API rằng mình gửi gì
    );
    header = curl_slist_append(
        header,
        "Accept: application/json"  // Accept để báo cho API rằng mình muốn nhận gì
    );
    curl_easy_setopt(
        curl,
        CURLOPT_URL, // Dán link nrgok đến Google Colab
        _baseURL.c_str()
    );
    curl_easy_setopt(
        curl,
        CURLOPT_POST, // 1 tương đương với true. Do HTTP mặc định là GET => Chuyển sang HTTP POST
        1
    );
    curl_easy_setopt(
        curl,
        CURLOPT_HTTPHEADER,
        header // Dán nhãn header nãy mình chuẩn bị vào
    );
    std::string payload_string = json_payload.dump(); // Chuyển json object -> json string (json string là cú phát trong file json bình thường)
    curl_easy_setopt(
        curl,
        CURLOPT_POSTFIELDS, // Chuyển json_payload mà mình chuẩn bị thành json string để chuẩn bị gửi
        payload_string.c_str() // Dùng c-string và libcurl (ngôn ngữ C) ko có std::string
    );
    // Do libcurl không biết mình làm gì với dữ liệu trả về và không biết dữ liệu đó sẽ gán vào đâu
    // => CURLOPT_WRITEFUNCTION sẽ giúp thực hiện hàm callBack khi dữ liệu được trả về
    // => Néu không có cái này, mặc định nó sẽ in dữ liệu ra console/Terminal của mình
    // Do có nhiều lần trả dữ liệu khác nhau nên phải có callBack để ghép dữ liệu
    curl_easy_setopt(
        curl,
        CURLOPT_WRITEFUNCTION,
        callBack
    );
    std::string responseData; 
    curl_easy_setopt(
        curl,
        CURLOPT_WRITEDATA, // Dùng để cho callback biết là ghi câu trả lời vào đâu
        &responseData
    );

    CURLcode result = curl_easy_perform(curl);

    curl_slist_free_all(header); // Giải phóng curl_slist*
    curl_easy_cleanup(curl); // Giải phóng CURL*

    // Libcurl là ngôn ngữ C nên ko xài try-catch được
    if(result != CURLE_OK){
        throw std::runtime_error("Libcurl Error: "+ std::string(curl_easy_strerror(result)));
    }

    try{
        json data = json::parse(responseData); // Chuyển json string về json obeject

        if(data.contains("error")){  // Xử lí khi Google Colab trả về json lỗi
            throw std::runtime_error("Ollama/FastAPI Error: " + data["error"].get<std::string>());
        }

        // Kĩ thuật C++20: Designated Initializers
        LLMResponse finalresponse = {
            .response = data["response"],
            .prompt_token = data["prompt_token"],
            .answer_token = data["answer_token"],
            .total_token = data["total_token"],
            .success = data["success"]
        };
        return finalresponse;
    }
    catch(const json::parse_error& e){
        throw std::runtime_error("Malformed JSON Error: " + std::string(e.what()) + "\nRaw Data to check: " + responseData);
    }
    catch(const std::exception& e){
        throw std::runtime_error("JSON Error: " + std::string(e.what()));
    }
}