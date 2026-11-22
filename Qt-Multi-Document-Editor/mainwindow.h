#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTextCharFormat>

namespace Ui { class MainWindow; }
class DocumentEditor;
class QFontComboBox;
class QSpinBox;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    DocumentEditor *newDocument();
    DocumentEditor *activeEditor() const;

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    void openDocument();
    void mergeFormat(const QTextCharFormat &format);
    void syncActions();
    void arrangeWindows(bool horizontal);
    void showFindDialog();
    Ui::MainWindow *ui;
    QFontComboBox *fontCombo;
    QSpinBox *fontSize;
    int count = 0;
};

#endif
