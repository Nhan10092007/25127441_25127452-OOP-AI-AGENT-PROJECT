#pragma once

#include<string>

class Evaluator{
public:
    virtual bool evaluate(const std::string& finalAnswer, const std::string& evalScript) = 0;
    virtual ~Evaluator() = default;
};