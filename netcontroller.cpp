#include "netcontroller.h"

netController::netController(QObject *parent) : QObject(parent)
{
    manager = new QNetworkAccessManager(this);
}

netController nc;
//std::vector<BoardTab*> bts;
//std::vector<Tab> tabs;
//std::vector<QWidget*> tabs;
