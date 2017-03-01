#include "netcontroller.h"

netController::netController(QObject *parent) : QObject(parent)
{
    manager = new QNetworkAccessManager(this);
}

netController nc;
