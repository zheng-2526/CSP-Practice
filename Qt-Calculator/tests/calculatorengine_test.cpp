#include "calculatorengine.h"

#include <cassert>
#include <cmath>

bool same(double x, double y) {
    return std::abs(x - y) < 1e-12;
}

int main() {
    double ans = 0.0;

    assert(CalculatorEngine::calculate(2.0, 3.0, CalculatorEngine::Operation::Add, ans));
    assert(same(ans, 5.0));

    assert(CalculatorEngine::calculate(8.0, 3.0, CalculatorEngine::Operation::Subtract, ans));
    assert(same(ans, 5.0));

    assert(CalculatorEngine::calculate(2.5, 4.0, CalculatorEngine::Operation::Multiply, ans));
    assert(same(ans, 10.0));

    assert(CalculatorEngine::calculate(9.0, 3.0, CalculatorEngine::Operation::Divide, ans));
    assert(same(ans, 3.0));

    assert(CalculatorEngine::calculate(10.0, 3.0, CalculatorEngine::Operation::Modulo, ans));
    assert(same(ans, 1.0));

    // 除数为 0 时，函数应返回 false，而不是让程序崩溃。
    assert(!CalculatorEngine::calculate(9.0, 0.0, CalculatorEngine::Operation::Divide, ans));
    assert(!CalculatorEngine::calculate(9.0, 0.0, CalculatorEngine::Operation::Modulo, ans));

    assert(CalculatorEngine::squareRoot(81.0, ans));
    assert(same(ans, 9.0));
    assert(!CalculatorEngine::squareRoot(-1.0, ans));

    assert(CalculatorEngine::reciprocal(4.0, ans));
    assert(same(ans, 0.25));
    assert(!CalculatorEngine::reciprocal(0.0, ans));

    const std::pair<const char *, double> examples[] = {
        {"2*(3+4)", 14}, {"(8-2)/3", 2}, {"2+3*4", 14},
        {"(2+3)*(4-(1+1))", 10}, {"-(2+3)*2", -10},
        {"2*-3", -6}, {"1.5*(2+.5)", 3.75},
        {"(10+7)%5", 2}, {"20/5*2", 8}, {"18%5*2", 6},
        {"sqrt(9)+1/(2+2)", 3.25}, {"sqrt(sqrt(81))", 3},
        {"1e+15*(2-1)", 1e15}, {" 2 * ( 3 + 4 ) ", 14},
        {"((-3))", -3}, {"1--2", 3}, {"2/-2", -1}
    };
    for(auto example : examples) {
        assert(CalculatorEngine::evaluate(example.first, ans));
        assert(same(ans, example.second));
    }
    const char *invalid[] = {
        "", "()", "(2+3", "2+3)", "2+", "2**3", "2(3)",
        "1/(2-2)", "3%0", "sqrt(-1)", "1..2", "1e", "abc", "1e9999"
    };
    for(auto s : invalid) assert(!CalculatorEngine::evaluate(s, ans));
    assert(!CalculatorEngine::evaluate(std::string(1100, '('), ans));
    return 0;
}
