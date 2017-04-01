#ifndef NETCONTROLLER_H
#define NETCONTROLLER_H

#include <QNetworkAccessManager>
#include <QNetworkCookieJar>
#include <QJsonObject>
#include "boardtab.h"
#include "filter.h"

class netController: public QObject  {
    Q_OBJECT
public:
    QNetworkAccessManager *manager;
    QNetworkAccessManager *jsonManager;
    QNetworkCookieJar *cookies;
    QJsonObject settingsFile;
    explicit netController(QObject * parent = nullptr);
    Filter *filter;
};

extern netController nc;
//extern std::vector<BoardTab*> bts;

struct Tab{
    enum TabType {Board,Thread} type;
    void* TabPointer;
    QString searchString;
};

struct Settings{
    QObject hidden;
};

//extern std::vector<Tab> tabs;

#endif // NETCONTROLLER_H
