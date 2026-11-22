#pragma once

#include "calculatorengine.h"

#include <QString>
#include <QWidget>

class QKeyEvent;

namespace Ui {
class MainWindow;
}

class MainWindow : public QWidget {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    void keyPressEvent(QKeyEvent *event) override;

private:
    void inputDigit(const QString &digit);
    void inputPoint();
    void inputParenthesis(bool opening);
    void chooseOperation(CalculatorEngine::Operation nextOp);
    void calculateResult();
    void changeSign();
    void inputSquareRoot();
    void inputReciprocal();
    void backspace();
    void clearAll();
    void updateDisplay();
    void applyUnary(const QString &prefix);
    int operandStart() const;
    bool evaluateExpression(double &ans) const;
    QString formatNumber(double x) const;
    QString operationText(CalculatorEngine::Operation op) const;
    void showError();

    Ui::MainWindow *ui;

    QString expression; // 保存完整表达式，按等号时才统一求值
    bool calculatorVisible = false;
    bool resultShown = false; // 显示结果后，输入数字开始新一轮计算
    bool error = false;
};
