#include "mainwindow.h"
#include <QApplication>
#include <QLocale>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // Ensure consistent number formatting (Dot as decimal separator)
    QLocale::setDefault(QLocale::C);

    MainWindow w;
    w.show();

    return app.exec();
}
