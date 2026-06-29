#include "calculator_tool.h"
#include <string>
#include <optional>
#include <vector>
#include <sstream>
#include <stack>

CalculatorTool::CalculatorTool() 
    : Tool("calculator", "Tool for performing basic arithmetic operations. Args parameter: a string containing the expression to calculate(example: '15 * 17')") {}

int getPriority(char op) {
    if (op == '+' || op == '-') return 1;
    if (op == '*' || op == '/') return 2;
    if (op == '^') return 3;
    return 0;
}
bool isNumberToken(const std::string& token) {
    if (token.empty()) return false;
    size_t i = 0;
    if (token[i] == '-' && token.size() > 1) {
        i = 1; // cho phép số âm dính liền dấu, ví dụ "-5"
    }
    bool hasDigit = false;
    for (; i < token.size(); ++i) {
        if (std::isdigit(static_cast<unsigned char>(token[i]))) {
            hasDigit = true;
        } else if (token[i] == '.') {
            continue; // cho phép số thực, ví dụ "3.14"
        } else {
            return false; 
        }
    }
    return hasDigit;
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
bool isValidOperator(char c) {
    return c == '+' || c == '-' || c == '*' || c == '/' || c == '^';
}
double operate(double a, double b, char op) {
    switch (op) {
        case '+': return a + b;
        case '-': return a - b;
        case '*': return a * b;
        case '/': 
            if (b == 0) throw std::runtime_error("Division by zero");
            return a / b;
        case '^': return pow(a, b);
        default: throw std::runtime_error("Unknown operator");
    }
}

double calculateExpression(const std::vector<std::string>&tokens){
    std::stack<double> numbers;
    std::stack<char> ops;
    for (const auto& token : tokens) {
        if (isNumberToken(token)) {
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
            if (token.size() != 1 || !isValidOperator(token[0])) {
                throw std::runtime_error("Unknown token '" + token + "'");
            }
            while (!ops.empty() && getPriority(ops.top()) >= getPriority(token[0])) {
                if (numbers.size() < 2) {
                    throw std::runtime_error("Malformed expression");
                }
                double b = numbers.top(); numbers.pop();
                double a = numbers.top(); numbers.pop();
                char op = ops.top(); ops.pop();
                numbers.push(operate(a, b, op));
            }
            ops.push(token[0]);
        }
    }
    while (!ops.empty()) {
        if (numbers.size() < 2) {
            throw std::runtime_error("Malformed expression");
        }
        double b = numbers.top(); numbers.pop();
        double a = numbers.top(); numbers.pop();
        char op = ops.top(); ops.pop();
        numbers.push(operate(a, b, op));
    }
    if (numbers.empty()) {
        throw std::runtime_error("Malformed expression");
    }
    return numbers.top();
}

std::string CalculatorTool::execute(const std::string& args) {
    if (args.empty()) {
        throw std::runtime_error("Missing 'expression'");
    }
    std::vector<std::string> tokens;
    tokenize(args, tokens);
    double result = calculateExpression(tokens);
    return std::to_string(result);
}