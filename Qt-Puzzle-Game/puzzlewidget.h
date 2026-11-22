#ifndef PUZZLEWIDGET_H
#define PUZZLEWIDGET_H

#include <QImage>
#include <QPoint>
#include <QWidget>

#include "puzzlemodel.h"

class PuzzleWidget : public QWidget {
    Q_OBJECT

public:
    explicit PuzzleWidget(QWidget *parent = nullptr);

    bool loadImage(const QString &fileName);
    void setImage(const QImage &image);
    void startNewGame();
    void setDifficulty(int n);
    QImage originalImage() const;

signals:
    void gamePrepared();
    void moveAttempted();
    void moveCountChanged(int count);
    void puzzleSolved();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    QRect boardRect() const;
    QRect tileRect(int index) const;
    int indexAt(const QPoint &point) const;
    QRect sourceRect(int tileNumber) const;
    QImage normalizedImage(const QImage &image) const;

    PuzzleModel model;
    QImage image;
    int dragIndex = -1;
    int targetIndex = -1;
    int moveCount = 0;
    bool dragging = false;
    QPoint dragPosition;
};

#endif
