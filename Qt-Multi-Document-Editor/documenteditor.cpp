#include "documenteditor.h"
#include "rtfcodec.h"

#include <QCloseEvent>
#include <QDir>
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
    QString suffix = QFileInfo(fileName).suffix().toLower();
    if(suffix == "rtf") {
        // 转换成功后再更新编辑区和文件路径。
        QTextDocument converted;
        QString error;
        QByteArray data = file.read(RtfCodec::maxFileSize + 1);
        bool ok = false;
        if(data.size() > RtfCodec::maxFileSize) error = QStringLiteral("RTF 文件超过 8 MB。");
        else if(file.error() != QFileDevice::NoError) error = file.errorString();
        else ok = RtfCodec::decode(data, converted, error);
        if(!ok) {
            QMessageBox::warning(this, QStringLiteral("打开失败"), error);
            return false;
        }
        setHtml(converted.toHtml());
    } else {
        QString text = QString::fromUtf8(file.readAll());
        if(suffix == "html" || suffix == "htm") setHtml(text);
        else setPlainText(text);
    }
    path = QFileInfo(fileName).absoluteFilePath();
    document()->setModified(false);
    updateTitle();
    return true;
}

bool DocumentEditor::saveFile(bool saveAs) {
    QString target = path;
    if(saveAs || target.isEmpty()) {
        const QString rtf = QStringLiteral("RTF 富文本 (*.rtf)");
        const QString html = QStringLiteral("HTML 文档 (*.html *.htm)");
        const QString txt = QStringLiteral("纯文本 UTF-8 (*.txt)");
        QStringList filters{html, txt};
        if(RtfCodec::available()) filters.prepend(rtf);
        QFileInfo info(target);
        QString suffix = info.suffix().toLower();
        QString filter = filters.first();
        if(suffix == "html" || suffix == "htm") filter = html;
        else if(suffix == "txt") filter = txt;
        QString suggested = target.isEmpty() ? QStringLiteral("文档%1").arg(number)
                                             : info.dir().filePath(info.completeBaseName());
        target = QFileDialog::getSaveFileName(this, QStringLiteral("保存文档"),
            suggested, filters.join(QStringLiteral(";;")), &filter);
        if(target.isEmpty()) return false;
        if(QFileInfo(target).suffix().isEmpty())
            target += filter == rtf ? ".rtf" : filter == txt ? ".txt" : ".html";
    }
    return writeFile(target);
}

bool DocumentEditor::writeFile(const QString &fileName) {
    QString suffix = QFileInfo(fileName).suffix().toLower();
    if(suffix != "rtf" && suffix != "txt" && suffix != "html" && suffix != "htm") {
        QMessageBox::warning(this, QStringLiteral("保存失败"),
                             QStringLiteral("请保存为 .rtf、.txt 或 .html 文件。"));
        return false;
    }
    QByteArray data;
    if(suffix == "rtf") {
        QString error;
        if(!RtfCodec::encode(*document(), data, error)) {
            QMessageBox::warning(this, QStringLiteral("保存失败"), error);
            return false;
        }
    } else {
        data = (suffix == "txt" ? toPlainText() : toHtml()).toUtf8();
    }
    QSaveFile file(fileName);
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
