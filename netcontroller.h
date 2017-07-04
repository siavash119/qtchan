#ifndef NETCONTROLLER_H
#define NETCONTROLLER_H

#include "filter.h"
#include <QNetworkAccessManager>
#include <QNetworkCookieJar>
#include <QNetworkDiskCache>

class netController: public QObject  {
	Q_OBJECT
public:
	QNetworkAccessManager *thumbManager;
	QNetworkAccessManager *fileManager;
	QNetworkAccessManager *jsonManager;
	QNetworkCookieJar *cookies;
	QNetworkDiskCache *diskCache;
	explicit netController(QObject *parent = Q_NULLPTR);
	Filter *filter;
};

extern netController nc;
//extern std::vector<BoardTab*> bts;

//extern std::vector<Tab> tabs;

#endif // NETCONTROLLER_H
