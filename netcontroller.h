#ifndef NETCONTROLLER_H
#define NETCONTROLLER_H

#include <QNetworkAccessManager>
#include <QNetworkCookieJar>
#include <QNetworkDiskCache>
#include <QNetworkProxy>

class netController: public QObject  {
	Q_OBJECT
public:
	QNetworkAccessManager *thumbManager;
	QNetworkAccessManager *fileManager;
	QNetworkAccessManager *jsonManager;
	QNetworkAccessManager *captchaManager;
	QNetworkAccessManager *postManager;
	QNetworkProxy proxy;
	QNetworkDiskCache *diskCache;
	explicit netController(QObject *parent = Q_NULLPTR);
	void loadCookies(QString passFile);
	void loadCookiesIntoAllManagers(QString domain, QString name, QString value);
	void removeCookies();
	void refreshManagers();
};

extern netController nc;

#endif // NETCONTROLLER_H
