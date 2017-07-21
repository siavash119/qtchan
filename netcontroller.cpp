#include "netcontroller.h"
#include <QDir>
#include <QNetworkCookie>
#include <QSettings>

netController::netController(QObject *parent) : QObject(parent)
{
	thumbManager = new QNetworkAccessManager(this);
	fileManager = new QNetworkAccessManager(this);
	jsonManager = new QNetworkAccessManager(this);

	cookies = new QNetworkCookieJar(this);
	//thumbManager->setCookieJar(cookies);
	//fileManager->setCookieJar(cookies);
	jsonManager->setCookieJar(cookies);

	diskCache = new QNetworkDiskCache(this);
	QDir().mkpath("cache");
	diskCache->setCacheDirectory("cache");
	//diskCache->setMaximumCacheSize(1073741824); //1GB cache
	jsonManager->setCache(diskCache);

	QDir().mkpath(QDir::homePath()+"/.config/qtchan");
	QString defaultCookies = QDir::homePath() + "/.config/qtchan/cookies";
	QSettings settings;
	QString cookiesFile = settings.value("cookiesFile",defaultCookies).toString();
	loadCookies(cookiesFile);
	//filter = new Filter();
}

//cookies include 4chan pass
void netController::loadCookies(QString passFile){
	QFile file;
	file.setFileName(passFile);
	if(file.open(QIODevice::ReadOnly | QIODevice::Text)){
		QTextStream in(&file);
		QString line;
		QStringList info;
		while(!in.atEnd()){
			line = in.readLine();
			if(line.startsWith("#") || !line.contains(":")) continue;
			info = line.split(":");
			qDebug() << "adding cookie: " << info.at(0);
			QNetworkCookie temp(info.at(0).toStdString().c_str(),
								info.at(1).toStdString().c_str());
			temp.setDomain(".4chan.org");
			temp.setSecure(1);
			temp.setPath("/");
			//thumbManager->cookieJar()->insertCookie(temp);
			//fileManager->cookieJar()->insertCookie(temp);
			jsonManager->cookieJar()->insertCookie(temp);
		}
	}
}

void netController::removeCookies(){
	delete jsonManager->cookieJar();
	jsonManager->setCookieJar(new QNetworkCookieJar());
}

netController nc;
//std::vector<BoardTab*> bts;
//std::vector<Tab> tabs;
//std::vector<QWidget*> tabs;
