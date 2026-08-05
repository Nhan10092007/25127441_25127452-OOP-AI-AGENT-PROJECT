#include"loop_detector.h"

LoopDetector::LoopDetector(const LoopThreshold& threshold): loopThreshold(threshold) {}

void LoopDetector::reset(){
    toolCallHistory.clear();
}

bool CallRecord::operator==(const CallRecord& other) const{
    return this->toolCall == other.toolCall && this->args == other.args;
}

bool CallRecord::operator!=(const CallRecord& other) const{
    return !(*this == other);
}

int LoopDetector::checkTrailingBlock(const int period) const{
    int repeatCount = 1;
    if(toolCallHistory.size() < period * 2){
        return repeatCount;
    }
    for(int i = toolCallHistory.size() - 1; i >= period; i -= period){
        if(toolCallHistory[i] == toolCallHistory[i - period]){
            if(period == 2){
                if(i - 1 - period >= 0 &&(toolCallHistory[i - 1] != toolCallHistory[i - 1 - period])){
                    break;
                }
            }
            ++repeatCount;
        }
        else{
            break;
        }
    }
    return repeatCount;
}

LoopStatus LoopDetector::record(const std::string& toolCall, const std::string& args){
    CallRecord newCall = {
        .toolCall = toolCall,
        .args = args
    };
    toolCallHistory.push_back(newCall);
    
    int genericRepeatCount = checkTrailingBlock(1);
    int pingPongCount = checkTrailingBlock(2);
    
    if(genericRepeatCount >= loopThreshold.repeatCritical || pingPongCount >= loopThreshold.pingpongCritical){
        return LoopStatus::CRITICAL;
    }
    else if(genericRepeatCount >= loopThreshold.repeatWarning || pingPongCount >= loopThreshold.pingpongWarning){
        return LoopStatus::WARNING;
    }
    return LoopStatus::OK;
}
