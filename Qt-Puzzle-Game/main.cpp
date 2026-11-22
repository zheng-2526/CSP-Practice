#include <QApplication>
#include <QProxyStyle>
#include <QStyleOption>

#include "mainwindow.h"

// 统一隐藏按钮和消息框按钮的焦点虚线，保留键盘操作能力。
class PlainStyle : public QProxyStyle {
public:
    void drawPrimitive(PrimitiveElement element, const QStyleOption *option,
                       QPainter *painter, const QWidget *widget = nullptr) const override {
        if(element == PE_FrameFocusRect) return;
        QProxyStyle::drawPrimitive(element, option, painter, widget);
    }
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setStyle(new PlainStyle);

    MainWindow window;
    window.show();

    return app.exec();
}
