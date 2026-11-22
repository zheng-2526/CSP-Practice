#include "mainwindow.h"
#include "documenteditor.h"
#include "rtfcodec.h"
#include "ui_mainwindow.h"

#include <QCloseEvent>
#include <QDialog>
#include <QFileDialog>
#include <QFontComboBox>
#include <QFontDialog>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QMdiSubWindow>
#include <QPushButton>
#include <QSignalBlocker>
#include <QStyleHints>
#include <QStyle>
#include <QToolButton>
#include <QGuiApplication>
#include <QSpinBox>
#include <QTextCursor>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
    // 使用和 PPT 相近的浅色系统界面，编辑区保持白底。
    QGuiApplication::styleHints()->setColorScheme(Qt::ColorScheme::Light);
    ui->setupUi(this);
    fontCombo = new QFontComboBox(this);
    // 过滤旧式位图字体，避免 DirectWrite 在字体预览时加载失败。
    fontCombo->setFontFilters(QFontComboBox::ScalableFonts);
    fontCombo->setObjectName("fontCombo");
    fontSize = new QSpinBox(this);
    fontSize->setObjectName("fontSize");
    fontSize->setRange(6, 96);
    fontSize->setValue(12);
    fontCombo->setToolTip(QStringLiteral("字体"));
    fontSize->setToolTip(QStringLiteral("字号（磅）"));
    ui->actionNew->setIcon(style()->standardIcon(QStyle::SP_FileIcon));
    ui->actionOpen->setIcon(style()->standardIcon(QStyle::SP_DialogOpenButton));
    ui->actionSave->setIcon(style()->standardIcon(QStyle::SP_DialogSaveButton));
    ui->toolBar->insertWidget(ui->actionBold, fontCombo);
    ui->toolBar->insertWidget(ui->actionBold, fontSize);
    const QList<QAction *> styleActions{ui->actionBold, ui->actionItalic, ui->actionUnderline};
    for(auto action : styleActions) {
        auto button = qobject_cast<QToolButton *>(ui->toolBar->widgetForAction(action));
        if(!button) continue;
        QFont font = button->font();
        font.setBold(action == ui->actionBold);
        font.setItalic(action == ui->actionItalic);
        font.setUnderline(action == ui->actionUnderline);
        button->setFont(font);
        button->setToolButtonStyle(Qt::ToolButtonTextOnly);
    }

    connect(ui->startButton, &QPushButton::clicked, this, [this]() {
        ui->pages->setCurrentWidget(ui->mdiArea);
        if(!activeEditor()) newDocument();
    });
    connect(ui->actionNew, &QAction::triggered, this, [this]() { newDocument(); });
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::openDocument);
    connect(ui->actionSave, &QAction::triggered, this, [this]() {
        if(auto editor = activeEditor()) editor->saveFile();
    });
    connect(ui->actionSaveAs, &QAction::triggered, this, [this]() {
        if(auto editor = activeEditor()) editor->saveFile(true);
    });
    connect(ui->actionClose, &QAction::triggered, ui->mdiArea, &QMdiArea::closeActiveSubWindow);
    connect(ui->actionExit, &QAction::triggered, this, &QWidget::close);
    connect(ui->actionWelcome, &QAction::triggered, this, [this]() {
        ui->pages->setCurrentWidget(ui->welcomePage);
    });
    connect(ui->actionAbout, &QAction::triggered, this, [this]() {
        QMessageBox::about(this, QStringLiteral("关于"),
            QStringLiteral("多文档文本编辑器\n作者：郑亚泽\n支持 TXT、HTML 和 Windows RTF 文档"));
    });

    // 每个编辑命令都作用于当前激活的子窗口。
    connect(ui->actionUndo, &QAction::triggered, this, [this]() { if(auto e = activeEditor()) e->undo(); });
    connect(ui->actionRedo, &QAction::triggered, this, [this]() { if(auto e = activeEditor()) e->redo(); });
    connect(ui->actionCut, &QAction::triggered, this, [this]() { if(auto e = activeEditor()) e->cut(); });
    connect(ui->actionCopy, &QAction::triggered, this, [this]() { if(auto e = activeEditor()) e->copy(); });
    connect(ui->actionPaste, &QAction::triggered, this, [this]() { if(auto e = activeEditor()) e->paste(); });
    connect(ui->actionSelectAll, &QAction::triggered, this, [this]() { if(auto e = activeEditor()) e->selectAll(); });
    connect(ui->actionFind, &QAction::triggered, this, &MainWindow::showFindDialog);
    connect(ui->actionBold, &QAction::triggered, this, [this](bool checked) {
        QTextCharFormat format;
        format.setFontWeight(checked ? QFont::Bold : QFont::Normal);
        mergeFormat(format);
    });
    connect(ui->actionItalic, &QAction::triggered, this, [this](bool checked) {
        QTextCharFormat format;
        format.setFontItalic(checked);
        mergeFormat(format);
    });
    connect(ui->actionUnderline, &QAction::triggered, this, [this](bool checked) {
        QTextCharFormat format;
        format.setFontUnderline(checked);
        mergeFormat(format);
    });
    connect(ui->actionFont, &QAction::triggered, this, [this]() {
        auto editor = activeEditor();
        if(!editor) return;
        bool ok = false;
        QFont font = QFontDialog::getFont(&ok, editor->currentFont(), this,
            QStringLiteral("设置字体"), QFontDialog::ScalableFonts);
        if(!ok) return;
        QTextCharFormat format;
        format.setFont(font);
        mergeFormat(format);
    });
    connect(fontCombo, &QFontComboBox::currentFontChanged, this, [this](const QFont &font) {
        QTextCharFormat format;
        format.setFontFamilies({font.family()});
        mergeFormat(format);
    });
    connect(fontSize, &QSpinBox::valueChanged, this, [this](int size) {
        QTextCharFormat format;
        format.setFontPointSize(size);
        mergeFormat(format);
    });
    connect(ui->actionLeft, &QAction::triggered, this, [this]() { if(auto e = activeEditor()) e->setAlignment(Qt::AlignLeft); });
    connect(ui->actionCenter, &QAction::triggered, this, [this]() { if(auto e = activeEditor()) e->setAlignment(Qt::AlignHCenter); });
    connect(ui->actionRight, &QAction::triggered, this, [this]() { if(auto e = activeEditor()) e->setAlignment(Qt::AlignRight); });
    connect(ui->actionCascade, &QAction::triggered, ui->mdiArea, &QMdiArea::cascadeSubWindows);
    connect(ui->actionHorizontal, &QAction::triggered, this, [this]() { arrangeWindows(true); });
    connect(ui->actionVertical, &QAction::triggered, this, [this]() { arrangeWindows(false); });
    connect(ui->mdiArea, &QMdiArea::subWindowActivated, this, [this]() { syncActions(); });
    syncActions();
}

MainWindow::~MainWindow() {
    delete ui;
}

DocumentEditor *MainWindow::activeEditor() const {
    auto window = ui->mdiArea->activeSubWindow();
    return window ? qobject_cast<DocumentEditor *>(window->widget()) : nullptr;
}

DocumentEditor *MainWindow::newDocument() {
    ui->pages->setCurrentWidget(ui->mdiArea);
    auto editor = new DocumentEditor(++count);
    auto window = ui->mdiArea->addSubWindow(editor);
    window->resize(600, 420);
    connect(editor, &QTextEdit::currentCharFormatChanged, this, [this]() { syncActions(); });
    connect(editor, &QTextEdit::cursorPositionChanged, this, [this]() { syncActions(); });
    editor->show();
    ui->mdiArea->setActiveSubWindow(window);
    editor->setFocus();
    syncActions();
    return editor;
}

void MainWindow::openDocument() {
    QString filter = RtfCodec::available()
        ? QStringLiteral("支持的文档 (*.rtf *.txt *.html *.htm);;RTF 富文本 (*.rtf)")
        : QStringLiteral("支持的文档 (*.txt *.html *.htm)");
    QString path = QFileDialog::getOpenFileName(this, QStringLiteral("打开文档"), QString(), filter);
    if(path.isEmpty()) return;
    auto editor = newDocument();
    if(!editor->loadFile(path)) editor->close();
}

void MainWindow::mergeFormat(const QTextCharFormat &format) {
    if(auto editor = activeEditor()) {
        // 有选区时修改选中的文字，没有选区时设置接下来输入的格式。
        editor->mergeCurrentCharFormat(format);
        editor->setFocus();
    }
}

void MainWindow::syncActions() {
    auto editor = activeEditor();
    const QList<QAction *> actions = {
        ui->actionSave, ui->actionSaveAs, ui->actionClose, ui->actionUndo,
        ui->actionRedo, ui->actionCut, ui->actionCopy, ui->actionPaste,
        ui->actionSelectAll, ui->actionFind, ui->actionFont, ui->actionBold,
        ui->actionItalic, ui->actionUnderline, ui->actionLeft, ui->actionCenter,
        ui->actionRight, ui->actionCascade, ui->actionHorizontal, ui->actionVertical
    };
    for(auto action : actions) action->setEnabled(editor != nullptr);
    fontCombo->setEnabled(editor != nullptr);
    fontSize->setEnabled(editor != nullptr);
    if(!editor) { ui->statusbar->showMessage(QStringLiteral("就绪")); return; }
    // 更新工具栏时阻断信号，防止切换文档意外改变其文字格式。
    QSignalBlocker blockFont(fontCombo), blockSize(fontSize);
    QFont font = editor->currentFont();
    fontCombo->setCurrentFont(font);
    fontSize->setValue(font.pointSize() > 0 ? font.pointSize() : 12);
    ui->actionBold->setChecked(font.bold());
    ui->actionItalic->setChecked(font.italic());
    ui->actionUnderline->setChecked(font.underline());
    QTextCursor cursor = editor->textCursor();
    ui->statusbar->showMessage(QStringLiteral("第 %1 行，第 %2 列    %3")
        .arg(cursor.blockNumber() + 1).arg(cursor.positionInBlock() + 1)
        .arg(editor->filePath().isEmpty() ? QStringLiteral("尚未保存") : editor->filePath()));
}

void MainWindow::arrangeWindows(bool horizontal) {
    auto windows = ui->mdiArea->subWindowList();
    int n = windows.size();
    if(n == 0) return;
    QRect area = ui->mdiArea->viewport()->rect();
    // 按比例计算边界，让最后一个窗口接住除法产生的余数。
    for(int i = 0; i < n; i++) {
        windows[i]->showNormal();
        if(horizontal)
            windows[i]->setGeometry(0, area.height() * i / n, area.width(),
                                   area.height() * (i + 1) / n - area.height() * i / n);
        else
            windows[i]->setGeometry(area.width() * i / n, 0,
                                   area.width() * (i + 1) / n - area.width() * i / n, area.height());
    }
}

void MainWindow::showFindDialog() {
    auto editor = activeEditor();
    if(!editor) return;
    QDialog dialog(this);
    dialog.setWindowTitle(QStringLiteral("查找和替换"));
    QFormLayout layout(&dialog);
    QLineEdit findText, replaceText;
    QPushButton findButton(QStringLiteral("查找下一个"));
    QPushButton replaceButton(QStringLiteral("替换当前"));
    QPushButton allButton(QStringLiteral("全部替换"));
    QLabel message;
    layout.addRow(QStringLiteral("查找："), &findText);
    layout.addRow(QStringLiteral("替换为："), &replaceText);
    layout.addRow(&findButton);
    layout.addRow(&replaceButton);
    layout.addRow(&allButton);
    layout.addRow(&message);
    auto findNext = [&]() {
        if(findText.text().isEmpty()) return;
        if(!editor->find(findText.text())) {
            editor->moveCursor(QTextCursor::Start);
            if(!editor->find(findText.text())) message.setText(QStringLiteral("未找到匹配文字"));
            else message.clear();
        } else message.clear();
    };
    connect(&findButton, &QPushButton::clicked, &dialog, findNext);
    connect(&replaceButton, &QPushButton::clicked, &dialog, [&]() {
        if(findText.text().isEmpty()) return;
        QTextCursor cursor = editor->textCursor();
        if(cursor.selectedText() == findText.text()) cursor.insertText(replaceText.text());
        findNext();
    });
    connect(&allButton, &QPushButton::clicked, &dialog, [&]() {
        if(findText.text().isEmpty()) return;
        int n = 0;
        QTextCursor cursor(editor->document());
        cursor.beginEditBlock();
        QTextCursor match = editor->document()->find(findText.text(), 0);
        while(!match.isNull()) {
            match.insertText(replaceText.text());
            n++;
            match = editor->document()->find(findText.text(), match);
        }
        cursor.endEditBlock();
        message.setText(QStringLiteral("已替换 %1 处").arg(n));
    });
    dialog.exec();
}

void MainWindow::closeEvent(QCloseEvent *event) {
    ui->mdiArea->closeAllSubWindows();
    if(!ui->mdiArea->subWindowList().isEmpty()) event->ignore();
    else event->accept();
}
