#ifndef NETCONTROLLER_H
#define NETCONTROLLER_H

#include <QNetworkAccessManager>

class netController: public QObject  {
    Q_OBJECT
public:
    QNetworkAccessManager *manager;
    explicit netController(QObject * parent = nullptr);
};

extern netController nc;

#endif // NETCONTROLLER_H
