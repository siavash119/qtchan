#include "netcontroller.h"
#include <QDir>
#include <QNetworkCookie>
#include <QSettings>
#include <QJsonDocument>
#include <QJsonObject>

netController::netController(QObject *parent) : QObject(parent)
{
	thumbManager = new QNetworkAccessManager(this);
	fileManager = new QNetworkAccessManager(this);
	jsonManager = new QNetworkAccessManager(this);

	cookies = new QNetworkCookieJar(this);
	thumbManager->setCookieJar(cookies);
	fileManager->setCookieJar(cookies);
	jsonManager->setCookieJar(cookies);

	diskCache = new QNetworkDiskCache(this);
	QDir().mkpath("cache");
	diskCache->setCacheDirectory("cache");
	//diskCache->setMaximumCacheSize(1073741824); //1GB cache
	jsonManager->setCache(diskCache);

	QString val;
	QFile file;
	QDir().mkpath(QDir::homePath()+"/.config/qtchan");
	file.setFileName(QDir::homePath() + "/.config/qtchan/settings");
	file.open(QIODevice::ReadOnly | QIODevice::Text);
	val = file.readAll();
	file.close();
	QJsonDocument d = QJsonDocument::fromJson(val.toUtf8());
	QJsonObject cookiesFile = d.object().value("cookies").toObject();
	//qDebug() << cookiesFile;
	for (QJsonObject::Iterator it=cookiesFile.begin(); it!=cookiesFile.end();it++) {
		QNetworkCookie temp(it.key().toStdString().c_str(),it.value().toString().toStdString().c_str());
		temp.setDomain(".4chan.org");
		temp.setSecure(1);
		temp.setPath("/");
		thumbManager->cookieJar()->insertCookie(temp);
		fileManager->cookieJar()->insertCookie(temp);
		jsonManager->cookieJar()->insertCookie(temp);
	}
	filter = new Filter();
}

netController nc;
//std::vector<BoardTab*> bts;
//std::vector<Tab> tabs;
//std::vector<QWidget*> tabs;
