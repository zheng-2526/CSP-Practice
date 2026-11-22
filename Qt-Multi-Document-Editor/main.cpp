#include <QApplication>
#include <QProxyStyle>
#include "mainwindow.h"

// 统一隐藏焦点虚线框，Tab、回车等键盘操作仍然可用。
class PlainStyle : public QProxyStyle {
public:
    void drawPrimitive(PrimitiveElement element, const QStyleOption *option,
                       QPainter *painter, const QWidget *widget = nullptr) const override {
        if(element != PE_FrameFocusRect)
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
