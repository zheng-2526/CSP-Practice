#include "calculatorengine.h"

#include <cmath>
#include <cctype>

namespace {

// 三层递归下降：expression 处理加减，term 处理乘除取模，factor 处理括号和数字。
// 每个字符只扫描有限次，时间复杂度 O(n)，嵌套括号占用 O(n) 栈空间。
struct Parser {
    const std::string &s;
    size_t i = 0;
    bool ok = true;

    void skipSpaces() {
        while(i < s.size() && std::isspace(static_cast<unsigned char>(s[i]))) i++;
    }

    bool take(char c) {
        skipSpaces();
        if(i >= s.size() || s[i] != c) return false;
        i++;
        return true;
    }

    double expression() {
        double x = term();
        while(ok) {
            CalculatorEngine::Operation op;
            if(take('+')) op = CalculatorEngine::Operation::Add;
            else if(take('-')) op = CalculatorEngine::Operation::Subtract;
            else break;
            double y = term(), ans = 0.0;
            if(ok) ok = CalculatorEngine::calculate(x, y, op, ans);
            x = ans;
        }
        return x;
    }

    double term() {
        double x = factor();
        while(ok) {
            CalculatorEngine::Operation op;
            if(take('*')) op = CalculatorEngine::Operation::Multiply;
            else if(take('/')) op = CalculatorEngine::Operation::Divide;
            else if(take('%')) op = CalculatorEngine::Operation::Modulo;
            else break;
            double y = factor(), ans = 0.0;
            if(ok) ok = CalculatorEngine::calculate(x, y, op, ans);
            x = ans;
        }
        return x;
    }

    double factor() {
        int sign = 1;
        // 一元正负号也可以放在括号前，例如 -(2+3)。
        while(true) {
            if(take('+')) continue;
            if(take('-')) sign = -sign;
            else break;
        }
        double x = 0.0;
        skipSpaces();
        if(s.compare(i, 4, "sqrt") == 0) {
            i += 4;
            if(!take('(')) { ok = false; return 0.0; }
            double y = expression();
            if(!take(')') || !CalculatorEngine::squareRoot(y, x)) ok = false;
        } else if(take('(')) {
            x = expression();
            if(!take(')')) ok = false;
        } else {
            size_t l = i;
            int digits = 0;
            while(i < s.size() && s[i] >= '0' && s[i] <= '9') { i++; digits++; }
            if(i < s.size() && s[i] == '.') {
                i++;
                while(i < s.size() && s[i] >= '0' && s[i] <= '9') { i++; digits++; }
            }
            if(digits == 0) { ok = false; return 0.0; }
            // 支持结果显示所使用的科学计数法，保证结果能继续参与运算。
            if(i < s.size() && (s[i] == 'e' || s[i] == 'E')) {
                i++;
                if(i < s.size() && (s[i] == '+' || s[i] == '-')) i++;
                size_t start = i;
                while(i < s.size() && s[i] >= '0' && s[i] <= '9') i++;
                if(i == start) { ok = false; return 0.0; }
            }
            try {
                x = std::stod(s.substr(l, i - l));
            } catch(...) {
                ok = false;
            }
        }
        return sign * x;
    }
};

}

bool CalculatorEngine::evaluate(const std::string &s, double &ans) {
    // 限制表达式长度，同时限制递归深度，避免过多括号导致栈溢出。
    if(s.empty() || s.size() > 1024) return false;
    Parser parser{s};
    double x = parser.expression();
    parser.skipSpaces();
    if(!parser.ok || parser.i != s.size() || !std::isfinite(x)) return false;
    ans = x;
    return true;
}

bool CalculatorEngine::calculate(double x, double y, Operation op, double &ans) {
    if(op == Operation::Add) ans = x + y;
    else if(op == Operation::Subtract) ans = x - y;
    else if(op == Operation::Multiply) ans = x * y;
    else if(op == Operation::Divide) {
        // 浮点数不能直接用 == 0 判断，这里给出一个很小的误差范围。
        if(std::abs(y) < 1e-15) return false;
        ans = x / y;
    } else if(op == Operation::Modulo) {
        if(std::abs(y) < 1e-15) return false;
        ans = std::fmod(x, y);
    } else return false;

    // 数值过大而溢出时，不把无穷大显示给用户。
    return std::isfinite(ans);
}

bool CalculatorEngine::squareRoot(double x, double &ans) {
    // 实数范围内，负数没有平方根。
    if(x < 0.0) return false;
    ans = std::sqrt(x);
    return std::isfinite(ans);
}

bool CalculatorEngine::reciprocal(double x, double &ans) {
    // 0 没有倒数。
    if(std::abs(x) < 1e-15) return false;
    ans = 1.0 / x;
    return std::isfinite(ans);
}
