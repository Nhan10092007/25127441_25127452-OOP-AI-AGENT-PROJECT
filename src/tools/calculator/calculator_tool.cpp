#include "calculator_tool.h"
#include <string>
#include <vector>
#include <stack>
#include <cmath>
#include <cctype>
#include <stdexcept>
#include<iomanip>
#include<sstream>

CalculatorTool::CalculatorTool() 
    : Tool("calculator", "Tool for performing basic arithmetic operations. Args parameter: a string containing the expression to calculate (supports +, -, *, /, ^ and parentheses). (example: '(2+3)^2*4-1')") {}

bool isValidOperator(char c) {
    return c == '+' || c == '-' || c == '*' || c == '/' || c == '^';
}

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
    size_t i = 0;
    while (i < expression.size()) {
        char c = expression[i];

        if (std::isspace(static_cast<unsigned char>(c))) {
            ++i;
            continue;
        }

        if (std::isdigit(static_cast<unsigned char>(c)) || c == '.') {
            std::string num;
            while (i < expression.size() &&
                   (std::isdigit(static_cast<unsigned char>(expression[i])) || expression[i] == '.')) {
                num += expression[i];
                ++i;
            }
            tokens.push_back(num);
        }
        else if (c == '(' || c == ')') {
            tokens.push_back(std::string(1, c));
            ++i;
        }
        else if (isValidOperator(c)) {
            // Xác định '-' là số âm nếu đứng ở đầu chuỗi, ngay sau '(' hoặc ngay sau 1 toán tử khác
            bool isNegative = (c == '-') &&
                (tokens.empty() || tokens.back() == "(" ||
                 (tokens.back().size() == 1 && isValidOperator(tokens.back()[0])));

            if (isNegative) {
                std::string num = "-";
                ++i;
                while (i < expression.size() &&
                       (std::isdigit(static_cast<unsigned char>(expression[i])) || expression[i] == '.')) {
                    num += expression[i];
                    ++i;
                }
                if (num == "-") {
                    throw std::runtime_error("Invalid negative number format");
                }
                tokens.push_back(num);
            } else {
                tokens.push_back(std::string(1, c));
                ++i;
            }
        }
        else {
            throw std::runtime_error(std::string("Unknown character: ") + c);
        }
    }
}

double operate(double a, double b, char op) {
    switch (op) {
        case '+': return a + b;
        case '-': return a - b;
        case '*': return a * b;
        case '/':
            if (b == 0) throw std::runtime_error("Division by zero");
            return a / b;
        case '^': return std::pow(a, b);
        default: throw std::runtime_error("Unknown operator");
    }
}

double calculateExpression(const std::vector<std::string>& tokens) {
    std::stack<double> numbers;
    std::stack<char> ops;

    for (const auto& token : tokens) {
        if (isNumberToken(token)) {
            numbers.push(std::stod(token));
        }
        else if (token == "(") {
            ops.push('(');
        }
        else if (token == ")") {
            while (!ops.empty() && ops.top() != '(') {
                if (numbers.size() < 2) {
                    throw std::runtime_error("Malformed expression");
                }
                double b = numbers.top(); numbers.pop();
                double a = numbers.top(); numbers.pop();
                char op = ops.top(); ops.pop();
                numbers.push(operate(a, b, op));
            }
            if (ops.empty()) {
                throw std::runtime_error("Mismatched parentheses");
            }
            ops.pop(); // pop the '('
        }
        else {
            if (token.size() != 1 || !isValidOperator(token[0])) {
                throw std::runtime_error("Unknown token '" + token + "'");
            }
            char currentOp = token[0];
            while (!ops.empty() && ops.top() != '(' && (getPriority(ops.top()) > getPriority(currentOp) || (getPriority(ops.top()) == getPriority(currentOp) && currentOp != '^'))){
                if (numbers.size() < 2) {
                    throw std::runtime_error("Malformed expression");
                }
                double b = numbers.top(); numbers.pop();
                double a = numbers.top(); numbers.pop();
                char op = ops.top(); ops.pop();
                numbers.push(operate(a, b, op));
            }
            ops.push(currentOp);
        }
    }

    while (!ops.empty()) {
        if (ops.top() == '(') {
            throw std::runtime_error("Mismatched parentheses");
        }
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
    result = std::round(result * 1e6) / 1e6;
    std::ostringstream oss;
    oss << std::setprecision(10) << result;
    return oss.str();
}