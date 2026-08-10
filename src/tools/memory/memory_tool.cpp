#include "memory_tool.h"
#include "nlohmann/json.hpp"
#include <string>
#include<vector>
#include<optional>
#include<algorithm>

using json = nlohmann::json;

static void helper(){
    sqlite3 *DB;
    int exit =0;
    exit = sqlite3_open("memory.db", &DB);
    if (exit) {
        throw std::runtime_error("Can't open database: " + std::string(sqlite3_errmsg(DB)));
    }
    sqlite3_busy_timeout(DB, 5000); // phòng SQLITE_BUSY khi có đụng độ truy cập

    std::string sql = "CREATE TABLE IF NOT EXISTS memory (key TEXT PRIMARY KEY, value TEXT, embedding TEXT);";
    char *errMsg;
    exit = sqlite3_exec(DB, sql.c_str(), nullptr, 0, &errMsg);
    if (exit != SQLITE_OK) {
        std::string errorMessage = "SQL error: " + std::string(errMsg);
        sqlite3_free(errMsg);
        sqlite3_close(DB);
        throw std::runtime_error(errorMessage);
    }
    sqlite3_close(DB);
}

static std::string embeddingToText(const std::vector<float>& embedding){
    json arr = embedding;
    return arr.dump();
}

static std::vector<float> textToEmbedding(const std::string& text){
    json arr = json::parse(text);
    return arr.get<std::vector<float>>();
}

static double cosineSimilarity(const std::vector<float>& a, const std::vector<float>& b){
    if(a.size() != b.size() || a.empty()){
        return 0.0;
    }
    double dot = 0.0, normA = 0.0, normB = 0.0;
    for(size_t i = 0; i < a.size(); ++i){
        dot += static_cast<double>(a[i]) * b[i];
        normA += static_cast<double>(a[i]) * a[i];
        normB += static_cast<double>(b[i]) * b[i];
    }
    if(normA == 0.0 || normB == 0.0){
        return 0.0; // tránh chia cho 0
    }
    return dot / (std::sqrt(normA) * std::sqrt(normB));
}

MemorySave::MemorySave(EmbeddingClient* client) 
    : Tool("memory_save", "Tool for saving a key-value pair to persistent memory. Retrieval later uses embedding-based SEMANTIC similarity. Args parameter: MUST be a JSON string with fields 'key' and 'value'. Example: {\"key\": \"username\", \"value\": \"john_doe\"}") ,
    embedClient(client)
{
    helper();
}
MemorySearch::MemorySearch(EmbeddingClient* client, double thresold) 
    : Tool("memory_search", "Tool for searching memory using SEMANTIC similarity (embedding-based) . The query may not need to match the saved key exactly - a closely related phrase or paraphrase can still find it. If unsure of the exact key, describe what you're looking for in a few words. Args parameter: a plain string query. Do NOT wrap it in JSON."),
    embedClient(client), similarityThreshold(thresold)
{
    helper();
}

std::string MemorySave::execute(const std::string& args) {
    json input = json::parse(args);

    if (!input.contains("key")|| !input.contains("value")){
        throw std::runtime_error("Missing required field 'key' or 'value'");
    }

    std::string key = input["key"].get<std::string>();
    std::string value = input["value"].get<std::string>();

    // Embed KEY (không phải value): giữ tương thích ngược với hành vi exact-match cũ,
    // vì query lúc search luôn được so với key, không phải value.
    std::vector<float> embedding = embedClient->embed(key);
    std::string embeddingText = embeddingToText(embedding);
    
    sqlite3 *DB;
    int exit = 0;
    exit = sqlite3_open("memory.db", &DB);
    if (exit) {
        throw std::runtime_error("Can't open database: " + std::string(sqlite3_errmsg(DB)));
    }
    sqlite3_busy_timeout(DB, 5000);

    sqlite3_stmt* stmt = nullptr;
    const char* sql = "INSERT OR REPLACE INTO memory (key, value, embedding) VALUES (?, ?, ?);";
    if (sqlite3_prepare_v2(DB, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::string err = sqlite3_errmsg(DB);
        sqlite3_close(DB);
        throw std::runtime_error("Failed to prepare statement: " + err);
    }
    sqlite3_bind_text(stmt, 1, key.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, value.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, embeddingText.c_str(), -1, SQLITE_TRANSIENT);
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::string err = sqlite3_errmsg(DB);
        sqlite3_finalize(stmt);
        sqlite3_close(DB);
        throw std::runtime_error("Failed to save to memory: " + err);
    }
    sqlite3_finalize(stmt);
    sqlite3_close(DB);
    return "Saved '" + key + "' to memory successfully.";
}

std::string MemorySearch::execute(const std::string&args){
    if (args.empty()) {
        throw std::runtime_error("Missing 'args' (key)");
    }
    std::vector<float> queryEmbedding = embedClient->embed(args);

    sqlite3 * DB;
    int exit = 0;
    exit = sqlite3_open("memory.db", &DB);
    if (exit) {
        throw std::runtime_error("Can't open database: " + std::string(sqlite3_errmsg(DB)));
    }
    sqlite3_busy_timeout(DB, 5000);

    const char* sql = "SELECT key, value, embedding FROM memory;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(DB, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::string err = sqlite3_errmsg(DB);
        sqlite3_close(DB);
        throw std::runtime_error("Failed to prepare statement: " + err);
    }
 
    // std::optional<T>: "best match hoặc không có gì tìm thấy"
    struct MatchCandidate { std::string key; std::string value; double similarity; };
    std::optional<MatchCandidate> best;
 
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::string rowKey = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        std::string rowValue = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        const unsigned char* embText = sqlite3_column_text(stmt, 2);
        if (!embText) continue; // entry cũ chưa có embedding (dữ liệu legacy trước khi thêm tính năng) -> bỏ qua
 
        std::vector<float> rowEmbedding = textToEmbedding(reinterpret_cast<const char*>(embText));
        double sim = cosineSimilarity(queryEmbedding, rowEmbedding);
 
        if (!best.has_value() || sim > best->similarity) {
            best = MatchCandidate{rowKey, rowValue, sim};
        }
    }
    sqlite3_finalize(stmt);
    sqlite3_close(DB);
 
    if (!best.has_value() || best->similarity < similarityThreshold) {
        return "Key not found"; // giữ nguyên văn bản cũ - memory_management.md đang dựa vào đúng câu này
    }
    return best->value;
}