#pragma once
#include "tools/tool.h"
#include"client/embed_client.h"
#include <sqlite3.h>
#include<optional>

class MemorySave : public Tool {
private:
    EmbeddingClient* embedClient;
public:
    MemorySave(EmbeddingClient* client);
    std::string execute(const std::string& args) override;
};

class MemorySearch : public Tool {
private:
    EmbeddingClient* embedClient;
    double similarityThreshold;
public:
    MemorySearch(EmbeddingClient* client, double threshold);
    std::string execute(const std::string& args) override;
};