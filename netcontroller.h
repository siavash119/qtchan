#ifndef NETCONTROLLER_H
#define NETCONTROLLER_H

#include <QNetworkAccessManager>
#include "boardtab.h"

class netController: public QObject  {
    Q_OBJECT
public:
    QNetworkAccessManager *manager;
    explicit netController(QObject * parent = nullptr);
};

extern netController nc;
//extern std::vector<BoardTab*> bts;

struct Tab{
    enum TabType {Board,Thread} type;
    void* TabPointer;
};

//extern std::vector<Tab> tabs;

#endif // NETCONTROLLER_H
