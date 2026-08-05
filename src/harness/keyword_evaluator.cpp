#include"keyword_evaluator.h"

KeywordEvaluator::~KeywordEvaluator() = default;

bool KeywordEvaluator::evaluate(const std::string& finalAnswer, const std::string& evalScript){
    if(evalScript.empty() || finalAnswer.empty()){
        return false;
    }
    return finalAnswer.contains(evalScript); // C++ 23: std::string::contains()
}