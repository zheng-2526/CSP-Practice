#include <QApplication>
#include <QAction>
#include <QFile>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QTemporaryDir>
#include <QTextDocument>
#include <QTimer>
#include <QMessageBox>
#include <QAbstractButton>
#include "mainwindow.h"
#include "documenteditor.h"

void check(bool result, const char *message) {
    if(!result) qFatal("%s", message);
}

int main(int argc, char *argv[]) {
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
    return 0;
}
