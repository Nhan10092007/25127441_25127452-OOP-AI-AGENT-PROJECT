#include "memory_tool.h"
#include "nlohmann/json.hpp"
#include <string>
using json = nlohmann::json;

void helper(){
    sqlite3 *DB;
        int exit =0;
        exit = sqlite3_open("memory.db", &DB);
        if (exit) {
            throw std::runtime_error("Error: Can't open database: " + std::string(sqlite3_errmsg(DB)));
        }
        std::string sql = "CREATE TABLE IF NOT EXISTS memory (key TEXT PRIMARY KEY, value TEXT);";
        char *errMsg;
        exit = sqlite3_exec(DB, sql.c_str(), nullptr, 0, &errMsg);
        if (exit != SQLITE_OK) {
            std::string errorMessage = "Error: SQL error: " + std::string(errMsg);
            sqlite3_free(errMsg);
            sqlite3_close(DB);
            throw std::runtime_error(errorMessage);
        }
        sqlite3_close(DB);
}
MemorySave::MemorySave() 
    : Tool("memory_save", "Save a key-value pair to memory. Args parameter: a JSON string with fields 'key' and 'value'. Example: {\"key\": \"username\", \"value\": \"john_doe\"}") {
        helper();
    }
MemorySearch::MemorySearch() 
    : Tool("memory_search", "Search for a value by its key. Args parameter: a plain string containing the key to look up (example: 'username'). Do NOT wrap it in JSON.")  {
        helper();
    }

std::string MemorySave::execute(const std::string& args) {
    json input;
    try {
        input = json::parse(args);
    } catch (const json::parse_error& e) {
        throw std::runtime_error(std::string("Error: Invalid JSON args - ") + e.what());
    }
    if (!input.contains("key")|| !input.contains("value")){
        throw std::runtime_error("Error: Missing required field 'key' or 'value'");
    }
    sqlite3 *DB;
    int exit =0;
    exit = sqlite3_open("memory.db", &DB);
    if (exit) {
        throw std::runtime_error("Error: Can't open database: " + std::string(sqlite3_errmsg(DB)));
    }
    std::string key = input["key"].get<std::string>();
    std::string value = input["value"].get<std::string>();
    sqlite3_stmt* stmt = nullptr;
    const char* sql = "INSERT OR REPLACE INTO memory (key, value) VALUES (?, ?);";
    sqlite3_prepare_v2(DB, sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, key.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, value.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    sqlite3_close(DB);
    "Saved '" + key + "' to memory successfully.";
}
std::string MemorySearch::execute(const std::string&args){
    if (args.empty()) {
        throw std::runtime_error("Error: Missing 'args' (key)");
    }
    sqlite3 * DB;
    int exit = 0;
    exit = sqlite3_open("memory.db", &DB);
    if (exit) {
        throw std::runtime_error("Error: Can't open database: " + std::string(sqlite3_errmsg(DB)));
    }
    std::string key=args;
    const char* sql = "SELECT value FROM memory WHERE key=?;";
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(DB, sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, key.c_str(), -1, SQLITE_TRANSIENT);
    std::string result;
    char *errMsg;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        result = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
    }
    else {
        result = "Key not found"; // Key not found
    }
    sqlite3_finalize(stmt);
    sqlite3_close(DB);
    return result;
}