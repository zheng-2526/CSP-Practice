#ifndef DOCUMENTEDITOR_H
#define DOCUMENTEDITOR_H

#include <QTextEdit>

class DocumentEditor : public QTextEdit {
    Q_OBJECT
public:
    explicit DocumentEditor(int n, QWidget *parent = nullptr);
    bool loadFile(const QString &path);
    bool saveFile(bool saveAs = false);
    bool writeFile(const QString &path);
    QString filePath() const;

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    void updateTitle();
    QString path;
    int number;
};

#endif
