#include "documenteditor.h"

#include <QCloseEvent>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QSaveFile>

DocumentEditor::DocumentEditor(int n, QWidget *parent)
    : QTextEdit(parent), number(n) {
    setAttribute(Qt::WA_DeleteOnClose);
    setFont(QFont(QStringLiteral("Microsoft YaHei"), 12));
    connect(document(), &QTextDocument::modificationChanged,
            this, [this]() { updateTitle(); });
    updateTitle();
}

QString DocumentEditor::filePath() const {
    return path;
}

void DocumentEditor::updateTitle() {
    QString name = path.isEmpty() ? QStringLiteral("文档%1").arg(number)
                                  : QFileInfo(path).fileName();
    setWindowTitle(name + (document()->isModified() ? QStringLiteral(" *") : QString()));
}

bool DocumentEditor::loadFile(const QString &fileName) {
    QFile file(fileName);
    if(!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, QStringLiteral("打开失败"), file.errorString());
        return false;
    }
    QString text = QString::fromUtf8(file.readAll());
    QString suffix = QFileInfo(fileName).suffix().toLower();
    if(suffix == "html" || suffix == "htm") setHtml(text);
    else setPlainText(text);
    path = QFileInfo(fileName).absoluteFilePath();
    document()->setModified(false);
    updateTitle();
    return true;
}

bool DocumentEditor::saveFile(bool saveAs) {
    QString target = path;
    if(saveAs || target.isEmpty()) {
        // HTML 可以保留字体等格式，TXT 只保存文字。
        QString filter = QStringLiteral("带格式文档 (*.html)");
        target = QFileDialog::getSaveFileName(this, QStringLiteral("保存文档"),
            target.isEmpty() ? QStringLiteral("文档%1.html").arg(number) : target,
            QStringLiteral("带格式文档 (*.html);;纯文本 UTF-8 (*.txt)"), &filter);
        if(target.isEmpty()) return false;
        if(QFileInfo(target).suffix().isEmpty())
            target += filter.contains("*.txt") ? ".txt" : ".html";
    }
    return writeFile(target);
}

bool DocumentEditor::writeFile(const QString &fileName) {
    QString suffix = QFileInfo(fileName).suffix().toLower();
    if(suffix != "txt" && suffix != "html" && suffix != "htm") {
        QMessageBox::warning(this, QStringLiteral("保存失败"),
                             QStringLiteral("请保存为 .txt 或 .html 文件。"));
        return false;
    }
    QSaveFile file(fileName);
    QByteArray data = (suffix == "txt" ? toPlainText() : toHtml()).toUtf8();
    // 先写临时文件，成功后替换目标，避免写入失败损坏已有文档。
    if(!file.open(QIODevice::WriteOnly) || file.write(data) != data.size() || !file.commit()) {
        QMessageBox::warning(this, QStringLiteral("保存失败"), file.errorString());
        return false;
    }
    path = QFileInfo(fileName).absoluteFilePath();
    document()->setModified(false);
    updateTitle();
    return true;
}

void DocumentEditor::closeEvent(QCloseEvent *event) {
    if(document()->isModified()) {
        auto result = QMessageBox::question(this, QStringLiteral("保存修改"),
            QStringLiteral("“%1”尚未保存，是否保存？").arg(windowTitle()),
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
            QMessageBox::Save);
        if(result == QMessageBox::Cancel || (result == QMessageBox::Save && !saveFile())) {
            event->ignore();
            return;
        }
    }
    event->accept();
}
