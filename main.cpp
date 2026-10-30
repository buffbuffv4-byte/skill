#include "MainWindow.h"
#include <QApplication>
#include <QFile>
#include <QMessageBox>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QApplication::setApplicationName("SkillOpti");
    QApplication::setApplicationVersion("0.1.0 BETA");
    QApplication::setOrganizationName("SkillOpti");
    QFile style(":/style.qss");
    if (style.open(QIODevice::ReadOnly | QIODevice::Text))
        app.setStyleSheet(QString::fromUtf8(style.readAll()));
    MainWindow window;
    window.show();
    return app.exec();
}
