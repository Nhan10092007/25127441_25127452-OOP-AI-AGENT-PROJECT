#pragma once

#include"evaluator.h"

class FunctionalEvaluator: public Evaluator{
public:
    FunctionalEvaluator(){}
    ~FunctionalEvaluator() override;
    bool evaluate(const std::string& finalAnswer, const std::string& evalScript) override;
};