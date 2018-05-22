#include "netcontroller.h"
#include <QDir>
#include <QStandardPaths>
#include <QNetworkCookie>
#include <QSettings>
#include <QStandardPaths>
#include <QDebug>

netController::netController(QObject *parent) : QObject(parent)
{
	//qt bug? AppConfigLocation is same as ConfigLocation
	//qDebug() << QStandardPaths::standardLocations(QStandardPaths::AppConfigLocation);
	thumbManager = new QNetworkAccessManager(this);
	fileManager = new QNetworkAccessManager(this);
	jsonManager = new QNetworkAccessManager(this);
	postManager = new QNetworkAccessManager(this);
	captchaManager = new QNetworkAccessManager(this);

	postManager->setCookieJar(new QNetworkCookieJar());

	diskCache = new QNetworkDiskCache(jsonManager);
	QDir().mkpath(QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/qtchan");
	diskCache->setCacheDirectory(QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/qtchan");
	//diskCache->setMaximumCacheSize(1073741824); //1GB cache
	jsonManager->setCache(diskCache);

	QString defaultCookies = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/qtchan/cookies";
	QSettings settings(QSettings::IniFormat,QSettings::UserScope,"qtchan","qtchan");
	QString cookiesFile = settings.value("cookiesFile",defaultCookies).toString();
	loadCookies(cookiesFile);

	if(settings.value("proxy/enable",false).toBool()==true){
		qDebug() << "PROXY HOST" << settings.value("proxy/host","127.0.0.1").toString();
		proxy.setType(QNetworkProxy::ProxyType(settings.value("proxy/type",1).toInt())); //QNetworkProxy::Socks5Proxy
		proxy.setHostName(settings.value("proxy/host","127.0.0.1").toString());
		proxy.setPort(settings.value("proxy/port",8080).toInt());
		if(!settings.value("proxy/user","").toString().isEmpty())
			proxy.setUser(settings.value("proxy/user","").toString());
		if(!settings.value("proxy/pass","").toString().isEmpty())
			proxy.setUser(settings.value("proxy/pass","").toString());
		captchaManager->setProxy(proxy);
	}
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
			qDebug().noquote() << "adding cookie" << info.at(0);
			QNetworkCookie temp(info.at(0).toStdString().c_str(),
								info.at(1).toStdString().c_str());
			temp.setDomain(".4chan.org");
			temp.setSecure(1);
			temp.setPath("/");
			postManager->cookieJar()->insertCookie(temp);
		}
	}
}

void netController::loadCookiesIntoAllManagers(QString domain, QString name, QString value){
	QNetworkCookie temp(name.toStdString().c_str(),value.toStdString().c_str());
	temp.setDomain(domain.toStdString().c_str());
	temp.setSecure(1);
	temp.setPath("/");
	if(!postManager){
		qDebug() << "can't set cookies: network access managers not initialized yet";
		return;
	}
	qDebug() << "adding cookie to" << domain << name << value;
	postManager->cookieJar()->insertCookie(temp);
	jsonManager->cookieJar()->insertCookie(temp);
	thumbManager->cookieJar()->insertCookie(temp);
	fileManager->cookieJar()->insertCookie(temp);
	captchaManager->cookieJar()->insertCookie(temp);
}

void netController::removeCookies(){
	delete postManager->cookieJar();
	postManager->setCookieJar(new QNetworkCookieJar());
}

void netController::refreshManagers(){
	postManager->setNetworkAccessible(QNetworkAccessManager::Accessible);
	jsonManager->setNetworkAccessible(QNetworkAccessManager::Accessible);
	thumbManager->setNetworkAccessible(QNetworkAccessManager::Accessible);
	fileManager->setNetworkAccessible(QNetworkAccessManager::Accessible);
	captchaManager->setNetworkAccessible(QNetworkAccessManager::Accessible);
}
//std::vector<BoardTab*> bts;
//std::vector<Tab> tabs;
//std::vector<QWidget*> tabs;
