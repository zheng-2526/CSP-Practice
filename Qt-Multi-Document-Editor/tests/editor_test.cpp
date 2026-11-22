#include <QApplication>
#include <QAction>
#include <QFile>
#include <QFileDialog>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QTemporaryDir>
#include <QTextDocument>
#include <QTextBlock>
#include <QTextCursor>
#include <QTimer>
#include <QMessageBox>
#include <QAbstractButton>
#include <cstdio>
#include <cstdlib>
#include "mainwindow.h"
#include "documenteditor.h"
#include "rtfcodec.h"

void check(bool result, const char *message) {
    if(!result) {
        std::fprintf(stderr, "%s\n", message);
        std::exit(1);
    }
}

int main(int argc, char *argv[]) {
    QApplication::setAttribute(Qt::AA_DontUseNativeDialogs);
    QApplication app(argc, argv);
    QTemporaryDir directory;
    check(directory.isValid(), "temporary directory");
    MainWindow window;
    window.show();
    auto first = window.newDocument();
    first->setPlainText(QStringLiteral("中文编辑测试\n第二行"));
    first->selectAll();
    window.findChild<QAction *>("actionBold")->trigger();
    check(first->currentFont().bold(), "bold selection");
    QString html = directory.filePath("sample.html");
    check(first->writeFile(html), "save HTML");
    check(!first->document()->isModified(), "saved modified state");

    auto second = window.newDocument();
    check(second->loadFile(html), "load HTML");
    check(second->toPlainText() == first->toPlainText(), "Chinese HTML round trip");
    second->selectAll();
    check(second->currentFont().bold(), "HTML preserves formatting");
    QString txt = directory.filePath("sample.txt");
    check(second->writeFile(txt), "save TXT");
    QFile file(txt);
    check(file.open(QIODevice::ReadOnly), "open saved TXT");
    check(QString::fromUtf8(file.readAll()) == first->toPlainText(), "Chinese TXT round trip");
    check(window.activeEditor() == second, "active document");
    window.findChild<QAction *>("actionItalic")->trigger();
    check(second->currentFont().italic(), "italic current document");
    check(!first->currentFont().italic(), "independent document formatting");
    auto area = window.findChild<QMdiArea *>("mdiArea");
    check(area->subWindowList().size() == 2, "multiple documents");
    window.findChild<QAction *>("actionHorizontal")->trigger();
    app.processEvents();
    auto windows = area->subWindowList();
    check(windows[1]->y() > windows[0]->y(), "horizontal tile");
    window.findChild<QAction *>("actionVertical")->trigger();
    check(windows[1]->x() > windows[0]->x(), "vertical tile");
    window.grab().save("editor-preview.png");
    // 模拟取消关闭，确认未保存文档不会被意外丢弃。
    QTimer::singleShot(0, []() {
        auto box = qobject_cast<QMessageBox *>(QApplication::activeModalWidget());
        check(box != nullptr, "unsaved prompt");
        box->button(QMessageBox::Cancel)->click();
    });
    check(!windows[1]->close(), "cancel prevents document close");
    check(area->subWindowList().size() == 2, "cancel preserves document");
    first->document()->setModified(false);
    second->document()->setModified(false);
    window.close();

#ifdef Q_OS_WIN
    check(RtfCodec::available(), "Windows Rich Edit available");
    QTextDocument external;
    QString error;
    const QByteArray fixture = R"({\rtf1\ansi\ansicpg1252\uc1{\fonttbl{\f0 Arial;}}{\colortbl;\red170\green20\blue40;}\f0\fs36\qc\b\cf1 \u20013?\u25991? \{RTF\} \\ caf\'e9\b0\par\qr\i\ul\fs24 Second\i0\ulnone})";
    check(RtfCodec::decode(fixture, external, error), qPrintable(error));
    check(external.toPlainText() == QStringLiteral("中文 {RTF} \\ café\nSecond"), "external RTF text");
    check(external.begin().blockFormat().alignment() == Qt::AlignHCenter, "external RTF center");
    check(external.lastBlock().blockFormat().alignment() == Qt::AlignRight, "external RTF right");
    QTextCursor cursor(&external);
    cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
    check(cursor.charFormat().fontPointSize() == 18 && cursor.charFormat().font().bold(), "external RTF bold and size");
    check(cursor.charFormat().foreground().color() == QColor(170, 20, 40), "external RTF color");
    cursor.setPosition(4);
    cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
    check(cursor.charFormat().font().family() == "Arial", qPrintable(
        QStringLiteral("external RTF font family is %1").arg(cursor.charFormat().font().family())));
    cursor.setPosition(external.lastBlock().position());
    cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
    check(cursor.charFormat().fontItalic() && cursor.charFormat().fontUnderline(), "external RTF italic and underline");
    check(!RtfCodec::decode("{\\rtf1 {broken}", external, error), "reject malformed RTF");
    check(!RtfCodec::decode("{\\rtf1 {\\pict AABB}}", external, error), "reject unsupported picture");

    DocumentEditor source(1), reopened(2);
    source.setHtml(external.toHtml());
    cursor = source.textCursor();
    cursor.movePosition(QTextCursor::End);
    cursor.insertText(QStringLiteral("\n\n含空行与中文 😊\n"));
    QString rtf = directory.filePath(QStringLiteral("文档1.rtf"));
    check(source.writeFile(rtf), "write RTF from editor");
    check(reopened.loadFile(rtf), "open saved RTF in editor");
    check(reopened.toPlainText() == source.toPlainText(), "RTF round trip preserves paragraphs");
    check(reopened.document()->begin().blockFormat().alignment() == Qt::AlignHCenter, "round trip paragraph format");
    cursor = reopened.textCursor();
    cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
    check(cursor.charFormat().font().bold() && cursor.charFormat().fontPointSize() == 18, "round trip character format");
    auto secondLine = reopened.document()->begin().next();
    check(secondLine.blockFormat().alignment() & Qt::AlignRight, "round trip second alignment");
    cursor.setPosition(secondLine.position());
    cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
    check(cursor.charFormat().fontItalic() && cursor.charFormat().fontUnderline(), "round trip italic and underline");
    reopened.moveCursor(QTextCursor::End);
    reopened.insertPlainText(QStringLiteral("再次保存"));
    check(reopened.saveFile(), "save back to same RTF");
    check(source.loadFile(rtf) && source.toPlainText() == reopened.toPlainText(), "reopen after save");

    MainWindow rtfWindow;
    rtfWindow.show();
    auto acceptFile = [](const QString &path, bool saving) {
        auto dialog = qobject_cast<QFileDialog *>(QApplication::activeModalWidget());
        check(dialog != nullptr, "show file dialog");
        const QString filter = dialog->selectedNameFilter();
        check(filter.contains("*.rtf"), "dialog offers RTF");
        if(saving) check(!filter.contains("*.html"), "new document defaults to RTF");
        dialog->selectFile(path);
        check(QMetaObject::invokeMethod(dialog, "accept"), "accept file dialog");
    };
    QTimer::singleShot(0, [&]() { acceptFile(rtf, false); });
    rtfWindow.findChild<QAction *>("actionOpen")->trigger();
    check(rtfWindow.activeEditor() && rtfWindow.activeEditor()->filePath() == rtf, "File Open loads RTF");
    auto newEditor = rtfWindow.newDocument();
    newEditor->setPlainText(QStringLiteral("首次保存"));
    QString newPath = directory.filePath("new-document");
    QTimer::singleShot(0, [&]() { acceptFile(newPath, true); });
    rtfWindow.findChild<QAction *>("actionSave")->trigger();
    check(newEditor->filePath() == newPath + ".rtf" && QFile::exists(newPath + ".rtf"),
          "first save adds the default RTF extension");
#endif
    return 0;
}
