#include"functional_evaluator.h"
#include<cstdio>

FunctionalEvaluator::~FunctionalEvaluator() = default;

bool FunctionalEvaluator::evaluate(const std::string& finalAnswer, const std::string& evalScript){
    if(evalScript.empty()){
        return false;
    }
    #ifdef _WIN32 // Windows
        FILE* pipe = _popen(evalScript.c_str(), "r");
    #else // Linux/Macos
        FILE* pipe = popen(evalScript.c_str(), "r");
    #endif

    if(!pipe){
        return false;
    }
    int exitCode = 0;
    #ifdef _WIN32
        exitCode = _pclose(pipe);
    #else
        exitCode = pclose(pipe);
    #endif

    return exitCode == 0;
}