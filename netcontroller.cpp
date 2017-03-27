#include "netcontroller.h"
#include <QNetworkCookie>
#include <QSettings>
#include <QJsonDocument>
#include <QJsonObject>

netController::netController(QObject *parent) : QObject(parent)
{
    manager = new QNetworkAccessManager(this);
    cookies = new QNetworkCookieJar(this);
    manager->setCookieJar(cookies);

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
    for (QJsonObject::Iterator it=cookiesFile.begin(); it!=cookiesFile.end();it++){
        QNetworkCookie temp(it.key().toStdString().c_str(),it.value().toString().toStdString().c_str());
        temp.setDomain(".4chan.org");
        temp.setSecure(1);
        temp.setPath("/");
        manager->cookieJar()->insertCookie(temp);
    }
    filter = new Filter();
    //settingsFile = d.object().value("settings").toObject();
}

netController nc;
//std::vector<BoardTab*> bts;
//std::vector<Tab> tabs;
//std::vector<QWidget*> tabs;
