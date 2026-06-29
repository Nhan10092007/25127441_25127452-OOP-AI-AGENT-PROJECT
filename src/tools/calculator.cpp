#include "calculator.h"
#include <string>
#include <optional>
#include <vector>
#include <sstream>
#include <stack>

CalculatorTool::CalculatorTool() 
    : Tool("calculator", "Tool for performing basic arithmetic operations. Args parameter: a string containing the expression to calculate(example: '2 + 3')") {}

int getPriority(char op) {
    if (op == '+' || op == '-') return 1;
    if (op == '*' || op == '/') return 2;
    if (op == '^') return 3;
    return 0;
}

void tokenize(const std::string& expression, std::vector<std::string>& tokens) {
    std::istringstream iss(expression);
    std::string token;
    while (iss >> token) {
        if (token != " ") {
            tokens.push_back(token);
        }
    }
};
double operate(double a, double b, char op) {
    switch (op) {
        case '+': return a + b;
        case '-': return a - b;
        case '*': return a * b;
        case '/': 
            if (b == 0) throw std::runtime_error("Error: Division by zero");
            return a / b;
        case '^': return pow(a, b);
        default: throw std::runtime_error("Error: Unknown operator");
    }
}

double calculateExpression(const std::vector<std::string>&tokens){
    std::stack<double> numbers;
    std::stack<char> ops;
    for (const auto& token : tokens) {
        if (isdigit(std::stod(token))){
            numbers.push(std::stod(token));
        }
        else if (token=="("){
            ops.push('(');
        }
        else if (token==")"){
            while (!ops.empty() && ops.top() != '(') {
                double b = numbers.top(); numbers.pop();
                double a = numbers.top(); numbers.pop();
                char op = ops.top(); ops.pop();
                numbers.push(operate(a, b, op));
            }
            if (!ops.empty()) ops.pop(); // pop the '('
        }
        else {
            while (!ops.empty() && getPriority(ops.top()) >= getPriority(token[0])) {
                double b = numbers.top(); numbers.pop();
                double a = numbers.top(); numbers.pop();
                char op = ops.top(); ops.pop();
                numbers.push(operate(a, b, op));
            }
            ops.push(token[0]);
        }
    }
    while (!ops.empty()) {
        double b = numbers.top(); numbers.pop();
        double a = numbers.top(); numbers.pop();
        char op = ops.top(); ops.pop();
        numbers.push(operate(a, b, op));
    }
    return numbers.top();
}

std::string CalculatorTool::execute(const std::string& args) {
    if (args.empty()) {
        throw std::runtime_error("Error: Missing 'expression'");
    }
    
    try {
        std::vector<std::string> tokens;
        tokenize(args, tokens);
        double result = calculateExpression(tokens);
        return std::to_string(result);
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("Error during calculation: ") + e.what());
    }
}
