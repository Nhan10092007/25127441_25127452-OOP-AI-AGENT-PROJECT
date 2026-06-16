#include "calculator.h"
#include <string>
#include <optional>
#include<vector>

CalculatorTool::CalculatorTool() 
    : Tool("Calculator", "Thực hiện phép tính cơ bản. Tham số JSON yêu cầu: 'operation' (+, -, *, /) và mảng 'operands' chứa các số.") {}

std::optional<std::string> CalculatorTool::execute(const nlohmann::json& args) {
    if (!args.contains("operation") || !args.contains("operands")) {
        return "Error: Missing 'operation' or 'operands'";
    }
    if (!args["operands"].is_array()) {
        return "Error: 'operands' must be an array of numbers";
    }
    try {
        std::string op = args["operation"];
        auto operands = args["operands"].get<std::vector<double>>();

        if (operands.empty()) {
            return "Error: 'operands' array is empty";
        }
        double result = operands[0];
        for (size_t i = 1; i < operands.size(); ++i) {
            if (op == "+") {
                result += operands[i];
            } 
            else if (op == "-") {
                result -= operands[i];
            } 
            else if (op == "*") {
                result *= operands[i];
            } 
            else if (op == "/") {
                if (operands[i] == 0) return "Error: Division by zero";
                result /= operands[i];
            } 
            else {
                return "Error: Unsupported operation '" + op + "'";
            }
        }

        return std::to_string(result);

    } catch (const std::exception& e) {
    
        return std::string("Error parsing arguments: ") + e.what();
    }
}
