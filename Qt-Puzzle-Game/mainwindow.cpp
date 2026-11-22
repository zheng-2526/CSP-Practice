#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDialog>
#include <QFileDialog>
#include <QLabel>
#include <QMessageBox>
#include <QPixmap>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    ui->pages->setCurrentWidget(ui->welcomePage);
    timer.setInterval(50);
    connect(&timer, &QTimer::timeout, this, &MainWindow::updateCountdown);
    connect(ui->puzzleBoard, &PuzzleWidget::gamePrepared, this, &MainWindow::prepareChallenge);
    connect(ui->beginChallengeButton, &QPushButton::clicked, this, &MainWindow::beginCountdown);
    connect(ui->puzzleBoard, &PuzzleWidget::moveAttempted, this, &MainWindow::updateCountdown);
    connect(ui->difficultyCombo, &QComboBox::currentIndexChanged, this, [this](int index) {
        ui->puzzleBoard->setDifficulty(index + 3);
    });

    connect(ui->startButton, &QPushButton::clicked, this, &MainWindow::startGame);
    connect(ui->newGameButton, &QPushButton::clicked,
            ui->puzzleBoard, &PuzzleWidget::startNewGame);
    connect(ui->shuffleButton, &QPushButton::clicked,
            ui->puzzleBoard, &PuzzleWidget::startNewGame);
    connect(ui->chooseImageButton, &QPushButton::clicked, this, &MainWindow::chooseImage);
    connect(ui->previewButton, &QPushButton::clicked, this, &MainWindow::showOriginalImage);
    connect(ui->backButton, &QPushButton::clicked, this, [this]() {
        timer.stop();
        playing = false;
        ui->puzzleBoard->setEnabled(false);
        ui->beginChallengeButton->setEnabled(false);
        ui->pages->setCurrentWidget(ui->welcomePage);
    });
    connect(ui->puzzleBoard, &PuzzleWidget::moveCountChanged,
            this, &MainWindow::updateMoveCount);
    connect(ui->puzzleBoard, &PuzzleWidget::puzzleSolved,
            this, &MainWindow::showSuccessMessage);

    ui->puzzleBoard->loadImage(QStringLiteral(":/images/puzzle_sample.png"));
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::showOriginalImage() {
    QImage image = ui->puzzleBoard->originalImage();
    if(image.isNull()) return;

    QDialog dialog(this);
    dialog.setWindowTitle(QStringLiteral("查看原图"));
    dialog.resize(560, 590);

    QVBoxLayout *layout = new QVBoxLayout(&dialog);
    QLabel *imageLabel = new QLabel(&dialog);
    imageLabel->setAlignment(Qt::AlignCenter);
    imageLabel->setPixmap(QPixmap::fromImage(image).scaled(
        530, 530, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    layout->addWidget(imageLabel);

    dialog.exec();
}

void MainWindow::chooseImage() {
    QString fileName = QFileDialog::getOpenFileName(
        this,
        QStringLiteral("选择拼图图片"),
        QString(),
        QStringLiteral("图片文件 (*.png *.jpg *.jpeg *.bmp)"));

    if(fileName.isEmpty()) return;

    if(!ui->puzzleBoard->loadImage(fileName)) {
        QMessageBox::warning(this,
                             QStringLiteral("读取失败"),
                             QStringLiteral("无法读取这张图片，请重新选择。"));
        return;
    }

    ui->statusLabel->setText(QStringLiteral("图片已切换，点击“开始挑战”后才能拖动"));
}

void MainWindow::startGame() {
    ui->pages->setCurrentWidget(ui->gamePage);
    ui->puzzleBoard->startNewGame();
}

void MainWindow::updateMoveCount(int count) {
    ui->moveLabel->setText(QStringLiteral("移动次数：%1").arg(count));
}

void MainWindow::showSuccessMessage() {
    if(!playing) return;
    if(elapsed.elapsed() >= 15000) { showFailureMessage(); return; }
    timer.stop();
    playing = false;
    ui->puzzleBoard->setEnabled(false);
    ui->statusLabel->setText(QStringLiteral("拼图完成！"));
    QMessageBox::information(this,
                             QStringLiteral("挑战成功"),
                             QStringLiteral("恭喜你完成了拼图！"));
}

void MainWindow::beginCountdown() {
    if(ui->pages->currentWidget() != ui->gamePage || playing ||
       !ui->beginChallengeButton->isEnabled()) return;
    playing = true;
    ui->beginChallengeButton->setEnabled(false);
    ui->puzzleBoard->setEnabled(true);
    elapsed.restart();
    ui->timeLabel->setText(QStringLiteral("剩余时间：15 秒"));
    ui->statusLabel->setText(QStringLiteral("请在 15 秒内完成拼图"));
    timer.start();
}

void MainWindow::prepareChallenge() {
    // 准备与开始分开：换图、切难度或重排都必须再次手动开始。
    timer.stop();
    playing = false;
    ui->puzzleBoard->setEnabled(false);
    ui->beginChallengeButton->setEnabled(true);
    ui->timeLabel->setText(QStringLiteral("剩余时间：15 秒"));
    ui->statusLabel->setText(QStringLiteral("准备就绪，点击“开始挑战”后才能拖动"));
}

void MainWindow::updateCountdown() {
    if(!playing) return;
    // 使用实际经过的毫秒数，打开原图窗口时倒计时也继续。
    qint64 remaining = qMax<qint64>(0, 15000 - elapsed.elapsed());
    ui->timeLabel->setText(QStringLiteral("剩余时间：%1 秒").arg((remaining + 999) / 1000));
    if(remaining == 0) showFailureMessage();
}

void MainWindow::showFailureMessage() {
    if(!playing) return;
    playing = false;
    timer.stop();
    ui->puzzleBoard->setEnabled(false);
    ui->statusLabel->setText(QStringLiteral("挑战失败：时间已到，请重新开始"));
    QMessageBox::information(this, QStringLiteral("挑战失败"),
                             QStringLiteral("15 秒已到，未完成拼图，挑战失败！"));
}
