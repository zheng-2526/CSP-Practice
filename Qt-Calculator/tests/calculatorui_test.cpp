#include "mainwindow.h"

#include <QApplication>
#include <QKeyEvent>
#include <QLabel>
#include <QPushButton>

void check(bool result, const char *message) {
    if(!result) qFatal("%s", message);
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    MainWindow window;
    window.show();
    app.processEvents();
    auto click = [&](const char *name) {
        auto button = window.findChild<QPushButton *>(name);
        check(button != nullptr, name);
        button->click();
    };
    auto display = window.findChild<QLabel *>("displayLabel");
    auto key = [&](int code) {
        QKeyEvent event(QEvent::KeyPress, code, Qt::NoModifier);
        QApplication::sendEvent(&window, &event);
    };
    auto enter = [&](const QString &text) {
        click("clearButton");
        for(auto c : text) key(c.unicode());
        click("equalButton");
    };
    click("startButton");
    enter("2*(3+4)");
    check(display->text() == "14", "keyboard parentheses");
    click("addButton");
    click("digit1Button");
    click("equalButton");
    check(display->text() == "15", "continue from result");
    enter("(2+3)*(4-(1+1))");
    check(display->text() == "10", "nested parentheses");
    enter("1/(2-2)");
    check(display->text() == QStringLiteral("错误"), "division by zero");
    click("clearButton");
    click("leftParenthesisButton");
    click("digit8Button");
    click("subtractButton");
    click("digit2Button");
    click("rightParenthesisButton");
    click("divideButton");
    click("digit3Button");
    click("equalButton");
    check(display->text() == "2", "mouse parentheses");
    enter("2(3+4)");
    check(display->text() == "14", "implicit multiplication input");
    enter("(17)%5");
    check(display->text() == "2", "modulo with parentheses");
    click("clearButton");
    click("digit9Button");
    click("rootButton");
    check(display->text() == "3", "square root preserved");
    click("reciprocalButton");
    click("equalButton");
    check(display->text().startsWith("0.33333"), "reciprocal preserved");
    click("clearButton");
    for(auto c : QString("2+(3*3)")) key(c.unicode());
    click("rootButton");
    click("equalButton");
    check(display->text() == "5", "square root of complete bracket operand");
    click("clearButton");
    for(auto c : QString("2*(3+")) key(c.unicode());
    click("digit9Button");
    click("rootButton");
    key(Qt::Key_ParenRight);
    click("equalButton");
    check(display->text() == "12", "square root inside unfinished outer bracket");
    click("clearButton");
    for(auto c : QString("(2+3")) key(c.unicode());
    click("equalButton");
    click("backButton");
    click("digit4Button");
    key(Qt::Key_ParenRight);
    click("equalButton");
    check(display->text() == "6", "correct expression after error with backspace");
    enter("-3");
    click("rootButton");
    check(display->text() == QStringLiteral("错误"), "negative square root");
    enter("(2+3");
    check(display->text() == QStringLiteral("错误"), "unmatched parentheses");
    key(Qt::Key_ParenRight);
    enter("2+3*4");
    check(display->text() == "14", "operator precedence");
    click("clearButton");
    for(auto c : QString("(2+3)*(4-(1+1))")) key(c.unicode());
    app.processEvents();
    window.grab().save("calculator-parentheses.png");
    click("equalButton");
    check(display->text() == "10", "final expression");
    return 0;
}
