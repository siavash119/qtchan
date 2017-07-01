#ifndef NETCONTROLLER_H
#define NETCONTROLLER_H

#include <QNetworkAccessManager>
#include <QNetworkCookieJar>
#include <QNetworkDiskCache>
#include <QJsonObject>
#include "boardtab.h"
#include "filter.h"

class netController: public QObject  {
	Q_OBJECT
public:
	QNetworkAccessManager *thumbManager;
	QNetworkAccessManager *fileManager;
	QNetworkAccessManager *jsonManager;
	QNetworkCookieJar *cookies;
	QNetworkDiskCache *diskCache;
	QJsonObject settingsFile;
	explicit netController(QObject *parent = Q_NULLPTR);
	Filter *filter;
};

extern netController nc;
//extern std::vector<BoardTab*> bts;

struct Settings{
	QObject hidden;
};

//extern std::vector<Tab> tabs;

#endif // NETCONTROLLER_H
