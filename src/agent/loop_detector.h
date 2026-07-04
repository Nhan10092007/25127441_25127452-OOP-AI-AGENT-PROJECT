#pragma once

#include<vector>
#include<string>

struct LoopThreshold{
    int repeatWarning = 2;
    int repeatCritical = 3;
    int pingpongWarning = 2;
    int pingpongCritical = 3;
};

struct CallRecord{
    std::string toolCall;
    std::string args;

    bool operator==(const CallRecord& other) const;
    bool operator!=(const CallRecord& other) const;
};

enum class LoopStatus{
    OK,
    WARNING, 
    CRITICAL
};

class LoopDetector{
private:
    std::vector<CallRecord> toolCallHistory;
    LoopThreshold loopThreshold;

    // Helper function:
    int checkTrailingBlock(const int period);
public:
    LoopDetector(const LoopThreshold& threshold = LoopThreshold{}); // Default argument
    LoopStatus record(const std::string& toolCall, const std::string& args);
    void reset();
};