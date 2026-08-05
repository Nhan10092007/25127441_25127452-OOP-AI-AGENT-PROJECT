#include"functional_evaluator.h"
#include<cstdio>
#include <array>  

FunctionalEvaluator::~FunctionalEvaluator() = default;

bool FunctionalEvaluator::evaluate(const std::string& finalAnswer, const std::string& evalScript){
    if(evalScript.empty()){
        return false;
    }
    std::array<char, 128> buffer;
    std::string result = "";
    #ifdef _WIN32 // Windows
        FILE* pipe = _popen(evalScript.c_str(), "r");
    #else // Linux/Macos
        FILE* pipe = popen(evalScript.c_str(), "r");
    #endif
    if(!pipe){
        return false;
    }
    while(fgets(buffer.data(), buffer.size(), pipe) != nullptr){
        result += buffer.data();
    }
    int exitCode = 0;
    #ifdef _WIN32
        exitCode = _pclose(pipe);
    #else
        exitCode = pclose(pipe);
    #endif

    return exitCode == 0;
}