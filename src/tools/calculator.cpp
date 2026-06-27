#include "calculator.h"
#include <string>
#include <optional>
#include<vector>

CalculatorTool::CalculatorTool() 
    : Tool("calculator", "Tool for performing basic arithmetic operations. Args parameter: a string containing the expression to calculate(example: '2 + 3')") {}

std::string CalculatorTool::execute(const std::string& args) {
    if (args.empty()) {
        throw std::runtime_error("Error: Missing 'args'");
    }

    try {
        std::istringstream iss(args);
        double result = 0;
        if (!(iss >> result)) {
            throw std::runtime_error("Error: Invalid start of expression");
        }
        char op;
        while (iss>>op){
            double nextValue;
            if (!(iss >> nextValue)) {
                throw std::runtime_error("Error: Missing number after operator");
            }
            switch(op){
                case '+': 
                result+=nextValue; 
                break;
            case '-': 
                result-=nextValue; 
                break;
            case '*': 
                result *=nextValue; 
                break;
            case '/': 
                if (nextValue == 0) {
                    throw std::runtime_error("Error: Can't divide for 0");
                }
                result/=nextValue; 
                break;
            default:
                throw std::runtime_error(std::string("Error operator: ") + op);
            }
        }
        return std::to_string(result);
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("Error during calculation: ") + e.what());
    }
}
