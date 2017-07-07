#include <QApplication>
#include "mainwindow.h"

MainWindow *mw;

int main(int argc, char *argv[])
{
	QCoreApplication::setOrganizationName("qtchan");
	QCoreApplication::setApplicationName("qtchan");
	QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
	QApplication a(argc, argv);
	MainWindow w;
	w.show();
	mw = &w;
	mw->loadSession();
	return a.exec();
}
