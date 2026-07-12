#pragma once

#include"evaluator.h"

class KeywordEvaluator: public Evaluator{
public:
    KeywordEvaluator(){}
    ~KeywordEvaluator() override;
    bool evaluate(const std::string& finalAnswer, const std::string& evalScript) override;
};