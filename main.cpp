#include "mainwindow.h"
#include <QApplication>
MainWindow *mw;

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName("anon");
    QCoreApplication::setApplicationName("qtchan");
    QApplication a(argc, argv);
    MainWindow w;
    mw = &w;
    w.show();
    return a.exec();
}
