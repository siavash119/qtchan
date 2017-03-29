#include <QApplication>
#include "mainwindow.h"
//#include <QSettings>
MainWindow *mw;

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName("qtchan");
    QCoreApplication::setApplicationName("qtchan");
    //QSettings settings;
    //settings.clear();
    QApplication a(argc, argv);
    MainWindow w;
    mw = &w;
    w.show();
    mw->loadSession();
    QObject::connect(&a, &QApplication::aboutToQuit, mw, &MainWindow::saveSession);
    return a.exec();
}
