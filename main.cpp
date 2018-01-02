#include "mainwindow.h"
#include <QApplication>
#include <QSettings>

MainWindow *mw;


int main(int argc, char *argv[])
{
	QCoreApplication::setOrganizationName("qtchan");
	QCoreApplication::setApplicationName("qtchan");
	//QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
	//QApplication::setAttribute(QT_SCALE_FACTOR,"1.4");
	//qputenv("QT_SCALE_FACTOR","0.2");
	QApplication a(argc, argv);
	QFont font = QGuiApplication::font();
	QSettings settings(QSettings::IniFormat,QSettings::UserScope,"qtchan","qtchan");
	font.setPointSize(settings.value("fontSize",14).toInt());
	a.setFont(font);
	MainWindow w;
	w.show();
	mw = &w;
	mw->loadSession();
	return a.exec();
}
