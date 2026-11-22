#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QElapsedTimer>
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void showOriginalImage();
    void chooseImage();
    void startGame();
    void updateMoveCount(int count);
    void showSuccessMessage();
    void beginCountdown();
    void prepareChallenge();
    void updateCountdown();
    void showFailureMessage();

    Ui::MainWindow *ui;
    QTimer timer;
    QElapsedTimer elapsed;
    bool playing = false;
};

#endif
