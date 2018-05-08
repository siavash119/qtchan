#include "mainwindow.h"
#include "notificationtray.h"
#include "netcontroller.h"
#include "chans.h"
#include <QApplication>
#include <QSettings>

MainWindow *mw;
NotificationView *nv;
netController nc;
Chan *fourChanAPI = new FourChan();
Chan *eightChanAPI = new EightChan();

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
	//flag path
	QDir().mkpath("flags/troll");
	MainWindow w;
	mw = &w;
	w.show();
	NotificationView view;
	nv = &view;
	NotificationTray t;
	t.setIcon(QIcon(":/icons/icon_22x22.png"));
	t.show();
	w.loadSession();
	return a.exec();
}
