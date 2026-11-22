#pragma once

#include <string>

class CalculatorEngine {
public:
    enum class Operation {
        None,
        Add,
        Subtract,
        Multiply,
        Divide,
        Modulo
    };

    // 用 x、y 表示两个操作数，ans 保存计算结果。
    static bool calculate(double x, double y, Operation op, double &ans);
    static bool squareRoot(double x, double &ans);
    static bool reciprocal(double x, double &ans);
    // 支持括号、运算优先级、正负号和 sqrt(...)，解析失败返回 false。
    static bool evaluate(const std::string &s, double &ans);
};
