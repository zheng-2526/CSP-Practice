#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFontMetrics>
#include <QKeyEvent>
#include <QPushButton>
#include <QRegularExpression>

#include <cmath>

namespace {
bool isOperator(QChar c) {
    return QStringLiteral("+−×÷%-").contains(c);
}
}

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    ui->pages->setCurrentWidget(ui->welcomePage);

    QPushButton *digitButtons[] = {
        ui->digit0Button, ui->digit1Button, ui->digit2Button,
        ui->digit3Button, ui->digit4Button, ui->digit5Button,
        ui->digit6Button, ui->digit7Button, ui->digit8Button,
        ui->digit9Button
    };
    for(auto *button : digitButtons) {
        connect(button, &QPushButton::clicked, this, [this, button]() {
            inputDigit(button->text());
        });
    }
    connect(ui->startButton, &QPushButton::clicked, this, [this]() {
        calculatorVisible = true;
        ui->pages->setCurrentWidget(ui->calculatorPage);
        setFocus();
    });
    connect(ui->pointButton, &QPushButton::clicked, this, &MainWindow::inputPoint);
    connect(ui->clearButton, &QPushButton::clicked, this, &MainWindow::clearAll);
    connect(ui->backButton, &QPushButton::clicked, this, &MainWindow::backspace);
    connect(ui->rootButton, &QPushButton::clicked, this, &MainWindow::inputSquareRoot);
    connect(ui->reciprocalButton, &QPushButton::clicked, this, &MainWindow::inputReciprocal);
    connect(ui->signButton, &QPushButton::clicked, this, &MainWindow::changeSign);
    connect(ui->equalButton, &QPushButton::clicked, this, &MainWindow::calculateResult);
    connect(ui->leftParenthesisButton, &QPushButton::clicked, this, [this]() { inputParenthesis(true); });
    connect(ui->rightParenthesisButton, &QPushButton::clicked, this, [this]() { inputParenthesis(false); });
    connect(ui->addButton, &QPushButton::clicked, this, [this]() {
        chooseOperation(CalculatorEngine::Operation::Add);
    });
    connect(ui->subtractButton, &QPushButton::clicked, this, [this]() {
        chooseOperation(CalculatorEngine::Operation::Subtract);
    });
    connect(ui->multiplyButton, &QPushButton::clicked, this, [this]() {
        chooseOperation(CalculatorEngine::Operation::Multiply);
    });
    connect(ui->divideButton, &QPushButton::clicked, this, [this]() {
        chooseOperation(CalculatorEngine::Operation::Divide);
    });
    connect(ui->modButton, &QPushButton::clicked, this, [this]() {
        chooseOperation(CalculatorEngine::Operation::Modulo);
    });
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::inputDigit(const QString &digit) {
    if(error || resultShown) clearAll();
    if(expression.size() >= 512) return;
    if(expression.endsWith(')')) expression += QChar(0x00d7);
    expression += digit;
    updateDisplay();
}

void MainWindow::inputPoint() {
    if(error || resultShown) clearAll();
    if(expression.size() >= 512) return;
    if(expression.endsWith(')')) expression += QChar(0x00d7);
    if(expression.isEmpty() || expression.endsWith('(') || isOperator(expression.back())) {
        expression += QStringLiteral("0.");
    } else {
        static const QRegularExpression number(QStringLiteral("[0-9.]+([eE][+-]?[0-9]+)?$"));
        auto match = number.match(expression);
        if(!match.hasMatch() || match.captured().contains('.') ||
           match.captured().contains('e', Qt::CaseInsensitive)) return;
        expression += '.';
    }
    updateDisplay();
}

void MainWindow::inputParenthesis(bool opening) {
    if(error || resultShown) clearAll();
    if(expression.size() >= 512) return;
    if(opening) {
        // 紧邻数字或右括号时自动补乘号，例如 2(3+4) 等价于 2×(3+4)。
        if(!expression.isEmpty() && (expression.back().isDigit() ||
           expression.endsWith(')') || expression.endsWith('.'))) expression += QChar(0x00d7);
        expression += '(';
    } else {
        if(expression.isEmpty() || expression.endsWith('(') || isOperator(expression.back())) return;
        if(expression.count('(') <= expression.count(')')) return;
        expression += ')';
    }
    updateDisplay();
}

void MainWindow::chooseOperation(CalculatorEngine::Operation nextOp) {
    if(error || expression.size() >= 512) return;
    resultShown = false;
    QString next = operationText(nextOp);
    if(expression.isEmpty() || expression.endsWith('(')) {
        if(nextOp != CalculatorEngine::Operation::Add &&
           nextOp != CalculatorEngine::Operation::Subtract) return;
    } else if(isOperator(expression.back())) {
        if(nextOp != CalculatorEngine::Operation::Subtract ||
           expression.endsWith(QChar(0x2212))) expression.chop(1);
    }
    expression += next;
    updateDisplay();
}

bool MainWindow::evaluateExpression(double &ans) const {
    QString s = expression;
    s.replace(QChar(0x2212), '-').replace(QChar(0x00d7), '*')
     .replace(QChar(0x00f7), '/').replace(QStringLiteral("√"), QStringLiteral("sqrt"));
    return CalculatorEngine::evaluate(s.toStdString(), ans);
}

void MainWindow::calculateResult() {
    if(error || expression.isEmpty()) return;
    double ans = 0.0;
    if(!evaluateExpression(ans)) { showError(); return; }
    QString formula = expression + QStringLiteral(" =");
    expression = formatNumber(ans);
    updateDisplay();
    ui->formulaLabel->setText(formula);
    ui->formulaLabel->setToolTip(formula);
    resultShown = true;
}

int MainWindow::operandStart() const {
    if(expression.isEmpty()) return -1;
    int l = -1;
    if(expression.endsWith(')')) {
        int depth = 0;
        for(int i = expression.size() - 1; i >= 0; i--) {
            if(expression[i] == ')') depth++;
            if(expression[i] == '(') depth--;
            if(depth == 0) { l = i; break; }
        }
        if(l > 0 && expression[l - 1] == QChar(0x221a)) l--;
    } else {
        static const QRegularExpression number(QStringLiteral("([0-9]+\\.?[0-9]*|\\.[0-9]+)([eE][+-]?[0-9]+)?$"));
        auto match = number.match(expression);
        if(match.hasMatch()) l = match.capturedStart();
    }
    // 将数字前的一元负号一起取出，保证 √(-3) 不会误算成 -√3。
    while(l > 0 && (expression[l - 1] == QChar(0x2212) ||
          expression[l - 1] == '-' || expression[l - 1] == '+') &&
          (l == 1 || expression[l - 2] == '(' || isOperator(expression[l - 2]))) l--;
    return l;
}

void MainWindow::applyUnary(const QString &prefix) {
    if(error) return;
    if(expression.isEmpty()) expression = QStringLiteral("0");
    int l = operandStart();
    if(l < 0) return;
    QString unary = prefix + '(' + expression.mid(l) + ')';
    QString s = unary;
    s.replace(QChar(0x2212), '-').replace(QChar(0x00d7), '*')
     .replace(QChar(0x00f7), '/').replace(QStringLiteral("√"), QStringLiteral("sqrt"));
    double ans = 0.0;
    if(!CalculatorEngine::evaluate(s.toStdString(), ans)) {
        showError();
        return;
    }
    expression = expression.left(l) + unary;
    resultShown = false;
    // 单独输入一个操作数时立即显示结果；括号中的操作仍保存在表达式里。
    if(l == 0) {
        expression = formatNumber(ans);
        updateDisplay();
        ui->formulaLabel->setText(unary);
        return;
    }
    updateDisplay();
}

void MainWindow::changeSign() {
    applyUnary(QStringLiteral("−"));
}

void MainWindow::inputSquareRoot() {
    applyUnary(QStringLiteral("√"));
}

void MainWindow::inputReciprocal() {
    applyUnary(QStringLiteral("1/"));
}

void MainWindow::backspace() {
    if(resultShown) return;
    error = false;
    expression.chop(1);
    updateDisplay();
}

void MainWindow::clearAll() {
    expression.clear();
    resultShown = false;
    error = false;
    updateDisplay();
}

void MainWindow::updateDisplay() {
    QString s = expression.isEmpty() ? QStringLiteral("0") : expression;
    ui->displayLabel->setStyleSheet(s.size() > 20 ? QStringLiteral("font-size: 18px;")
                                                           : QStringLiteral("font-size: 28px;"));
    QFontMetrics metrics(ui->displayLabel->font());
    ui->displayLabel->setText(metrics.elidedText(s, Qt::ElideLeft, ui->displayLabel->width() - 24));
    ui->displayLabel->setToolTip(s);
    ui->formulaLabel->clear();
    ui->formulaLabel->setToolTip(QString());
}

QString MainWindow::formatNumber(double x) const {
    if(std::abs(x) < 1e-15) x = 0.0;
    return QString::number(x, 'g', 15);
}

QString MainWindow::operationText(CalculatorEngine::Operation op) const {
    if(op == CalculatorEngine::Operation::Add) return QStringLiteral("+");
    if(op == CalculatorEngine::Operation::Subtract) return QStringLiteral("−");
    if(op == CalculatorEngine::Operation::Multiply) return QStringLiteral("×");
    if(op == CalculatorEngine::Operation::Divide) return QStringLiteral("÷");
    if(op == CalculatorEngine::Operation::Modulo) return QStringLiteral("%");
    return QString();
}

void MainWindow::showError() {
    ui->formulaLabel->setText(QStringLiteral("请检查括号、表达式或除数（退格可修改）"));
    ui->displayLabel->setText(QStringLiteral("错误"));
    ui->displayLabel->setToolTip(expression);
    resultShown = false;
    error = true;
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
    if(!calculatorVisible) { QWidget::keyPressEvent(event); return; }
    const int key = event->key();
    if(key >= Qt::Key_0 && key <= Qt::Key_9) inputDigit(QString::number(key - Qt::Key_0));
    else if(key == Qt::Key_Period || key == Qt::Key_Comma) inputPoint();
    else if(key == Qt::Key_ParenLeft) inputParenthesis(true);
    else if(key == Qt::Key_ParenRight) inputParenthesis(false);
    else if(key == Qt::Key_Plus) chooseOperation(CalculatorEngine::Operation::Add);
    else if(key == Qt::Key_Minus) chooseOperation(CalculatorEngine::Operation::Subtract);
    else if(key == Qt::Key_Asterisk) chooseOperation(CalculatorEngine::Operation::Multiply);
    else if(key == Qt::Key_Slash) chooseOperation(CalculatorEngine::Operation::Divide);
    else if(key == Qt::Key_Enter || key == Qt::Key_Return || key == Qt::Key_Equal) calculateResult();
    else if(key == Qt::Key_Backspace) backspace();
    else if(key == Qt::Key_Escape || key == Qt::Key_Delete) clearAll();
    else if(key == Qt::Key_Percent) chooseOperation(CalculatorEngine::Operation::Modulo);
    else QWidget::keyPressEvent(event);
}
