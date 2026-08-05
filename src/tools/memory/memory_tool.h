#pragma once
#include "tool.h"
#include <sqlite3.h>

class MemorySave : public Tool {
public:
    MemorySave();
   std::string execute(const std::string& args) override;
};

class MemorySearch : public Tool {
public:
    MemorySearch();
    std::string execute(const std::string& args) override;
};