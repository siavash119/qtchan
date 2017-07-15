#include <QApplication>
#include "mainwindow.h"

MainWindow *mw;

int main(int argc, char *argv[])
{
	QCoreApplication::setOrganizationName("qtchan");
	QCoreApplication::setApplicationName("qtchan");
	//QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
	//QApplication::setAttribute(QT_SCALE_FACTOR,)
	//qputenv("QT_SCALE_FACTOR","1.4");
	QApplication a(argc, argv);
	QFont font = QGuiApplication::font();
	font.setPointSize(14);
	a.setFont(font);
	MainWindow w;
	w.show();
	mw = &w;
	mw->loadSession();
	return a.exec();
}
