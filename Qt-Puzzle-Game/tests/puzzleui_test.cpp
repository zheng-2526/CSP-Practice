#include "mainwindow.h"
#include "puzzlewidget.h"

#include <QApplication>
#include <QComboBox>
#include <QEventLoop>
#include <QLabel>
#include <QMouseEvent>
#include <QPushButton>
#include <QTimer>

void check(bool result, const char *message) {
    if(!result) qFatal("%s", message);
}

void waitFor(int ms) {
    QEventLoop loop;
    QTimer::singleShot(ms, &loop, &QEventLoop::quit);
    loop.exec();
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    MainWindow window;
    window.show();
    app.processEvents();
    auto board = window.findChild<PuzzleWidget *>("puzzleBoard");
    auto start = window.findChild<QPushButton *>("beginChallengeButton");
    auto time = window.findChild<QLabel *>("timeLabel");
    auto moves = window.findChild<QLabel *>("moveLabel");
    auto difficulty = window.findChild<QComboBox *>("difficultyCombo");
    check(board && start && time && moves && difficulty, "UI controls");
    window.findChild<QPushButton *>("startButton")->click();
    check(!board->isEnabled() && start->isEnabled(), "prepared and locked");

    auto drag = [&]() {
        int side = qMin(board->width(), board->height()) - 8;
        side -= side % 3;
        int x = (board->width() - side) / 2, y = (board->height() - side) / 2;
        QPointF a(x + side / 6, y + side / 6);
        QPointF b(x + side / 2, y + side / 6);
        QMouseEvent press(QEvent::MouseButtonPress, a, a, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        QMouseEvent release(QEvent::MouseButtonRelease, b, b, Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
        QApplication::sendEvent(board, &press);
        QApplication::sendEvent(board, &release);
    };
    drag();
    check(moves->text() == QStringLiteral("移动次数：0"), "cannot move before start");
    waitFor(1200);
    check(time->text() == QStringLiteral("剩余时间：15 秒"), "no automatic countdown");

    start->click();
    check(board->isEnabled() && !start->isEnabled(), "manual start unlocks board");
    waitFor(1200);
    check(time->text() != QStringLiteral("剩余时间：15 秒"), "countdown after start");
    drag();
    check(moves->text() == QStringLiteral("移动次数：1"), "dragging while playing");
    window.findChild<QPushButton *>("shuffleButton")->click();
    check(!board->isEnabled() && start->isEnabled(), "shuffle waits again");
    waitFor(1200);
    check(time->text() == QStringLiteral("剩余时间：15 秒"), "shuffle stops timer");
    start->click();
    difficulty->setCurrentIndex(1);
    check(!board->isEnabled() && start->isEnabled(), "difficulty waits again");
    check(moves->text() == QStringLiteral("移动次数：0"), "difficulty resets moves");
    start->click();
    QImage image(90, 90, QImage::Format_RGB32);
    image.fill(Qt::white);
    board->setImage(image);
    check(!board->isEnabled() && start->isEnabled(), "new image waits again");
    check(time->text() == QStringLiteral("剩余时间：15 秒"), "new image resets time");
    // 恢复示例图片，保存新版控件布局用于检查。
    board->loadImage(QStringLiteral(":/images/puzzle_sample.png"));
    app.processEvents();
    window.grab().save("puzzle-ready-preview.png");
    return 0;
}
