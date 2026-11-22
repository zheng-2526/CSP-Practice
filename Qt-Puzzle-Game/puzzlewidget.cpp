#include "puzzlewidget.h"

#include <QMouseEvent>
#include <QPainter>

PuzzleWidget::PuzzleWidget(QWidget *parent) : QWidget(parent) {
    setMinimumSize(360, 360);
    setMouseTracking(true);
}

bool PuzzleWidget::loadImage(const QString &fileName) {
    QImage loadedImage(fileName);
    if(loadedImage.isNull()) return false;

    setImage(loadedImage);
    return true;
}

void PuzzleWidget::setImage(const QImage &newImage) {
    if(newImage.isNull()) return;
    image = normalizedImage(newImage);
    startNewGame();
}

void PuzzleWidget::startNewGame() {
    // 打乱只是准备阶段，等待用户点击“开始挑战”后才允许拖动。
    setEnabled(false);
    model.reset();
    model.shuffle();
    dragIndex = -1;
    targetIndex = -1;
    moveCount = 0;
    dragging = false;
    emit moveCountChanged(moveCount);
    update();
    emit gamePrepared();
}

void PuzzleWidget::setDifficulty(int n) {
    model.setDifficulty(n);
    startNewGame();
}

QImage PuzzleWidget::originalImage() const {
    return image;
}

void PuzzleWidget::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    painter.fillRect(rect(), QColor(248, 248, 248));

    QRect area = boardRect();
    painter.fillRect(area, Qt::white);

    if(image.isNull()) {
        painter.setPen(QColor(100, 100, 100));
        painter.drawText(area, Qt::AlignCenter, QStringLiteral("请选择一张图片"));
        return;
    }

    // 按 order 中记录的顺序，把原图对应区域绘制到九宫格中。
    for(int i = 0; i < model.tileCount; i++) {
        if(dragging && i == dragIndex) continue;

        QRect destination = tileRect(i);
        painter.drawImage(destination, image, sourceRect(model.tileAt(i)));
        painter.setPen(QPen(QColor(110, 110, 110), 1));
        painter.drawRect(destination.adjusted(0, 0, -1, -1));
    }

    if(targetIndex >= 0 && dragging) {
        painter.fillRect(tileRect(targetIndex), QColor(60, 130, 220, 55));
    }

    if(dragging && dragIndex >= 0) {
        QRect piece = tileRect(dragIndex);
        piece.moveCenter(dragPosition);
        painter.setOpacity(0.85);
        painter.drawImage(piece, image, sourceRect(model.tileAt(dragIndex)));
        painter.setOpacity(1.0);
        painter.setPen(QPen(QColor(45, 105, 180), 2));
        painter.drawRect(piece.adjusted(0, 0, -1, -1));
    }

    painter.setPen(QPen(QColor(70, 70, 70), 2));
    painter.drawRect(area.adjusted(0, 0, -1, -1));
}

void PuzzleWidget::mousePressEvent(QMouseEvent *event) {
    if(!isEnabled() || event->button() != Qt::LeftButton || image.isNull()) return;

    dragIndex = indexAt(event->position().toPoint());
    if(dragIndex < 0) return;

    dragPosition = event->position().toPoint();
    dragging = true;
    update();
}

void PuzzleWidget::mouseMoveEvent(QMouseEvent *event) {
    if(!isEnabled() || !dragging) return;

    dragPosition = event->position().toPoint();
    targetIndex = indexAt(dragPosition);
    update();
}

void PuzzleWidget::mouseReleaseEvent(QMouseEvent *event) {
    // 在交换前检查真实用时，避免定时器刷新间隙内超时仍算成功。
    emit moveAttempted();
    if(!isEnabled()) { dragging = false; update(); return; }
    if(event->button() != Qt::LeftButton || !dragging) return;

    int releaseIndex = indexAt(event->position().toPoint());
    bool changed = releaseIndex >= 0 && releaseIndex != dragIndex;

    // 松开鼠标时交换起点和终点，而不是移动控件本身。
    if(changed) {
        model.swapTiles(dragIndex, releaseIndex);
        moveCount++;
        emit moveCountChanged(moveCount);
    }

    dragIndex = -1;
    targetIndex = -1;
    dragging = false;
    update();

    if(changed && model.isSolved()) emit puzzleSolved();
}

QRect PuzzleWidget::boardRect() const {
    int side = qMin(width(), height()) - 8;
    side -= side % model.rowCount;
    int left = (width() - side) / 2;
    int top = (height() - side) / 2;
    return QRect(left, top, side, side);
}

QRect PuzzleWidget::tileRect(int index) const {
    QRect area = boardRect();
    int tileSize = area.width() / model.rowCount;
    int row = index / model.rowCount;
    int column = index % model.rowCount;
    return QRect(area.left() + column * tileSize,
                 area.top() + row * tileSize,
                 tileSize,
                 tileSize);
}

int PuzzleWidget::indexAt(const QPoint &point) const {
    QRect area = boardRect();
    if(!area.contains(point)) return -1;

    int tileSize = area.width() / model.rowCount;
    int column = (point.x() - area.left()) / tileSize;
    int row = (point.y() - area.top()) / tileSize;
    return row * model.rowCount + column;
}

QRect PuzzleWidget::sourceRect(int tileNumber) const {
    // 根据拼图块编号，计算它在完整原图中的裁剪区域。
    int tileSize = image.width() / model.rowCount;
    int row = tileNumber / model.rowCount;
    int column = tileNumber % model.rowCount;
    return QRect(column * tileSize, row * tileSize, tileSize, tileSize);
}

QImage PuzzleWidget::normalizedImage(const QImage &sourceImage) const {
    // 先把任意尺寸图片等比例放进正方形画布，之后才能均匀切成 3×3。
    const int imageSize = 900;
    QImage canvas(imageSize, imageSize, QImage::Format_ARGB32_Premultiplied);
    canvas.fill(Qt::white);

    QImage scaledImage = sourceImage.scaled(imageSize, imageSize,
                                            Qt::KeepAspectRatio,
                                            Qt::SmoothTransformation);
    QPainter painter(&canvas);
    int x = (imageSize - scaledImage.width()) / 2;
    int y = (imageSize - scaledImage.height()) / 2;
    painter.drawImage(x, y, scaledImage);
    return canvas;
}
