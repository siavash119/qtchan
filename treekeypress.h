#ifndef TREEKEYPRESS_H
#define TREEKEYPRESS_H

#include <QObject>
#include "mainwindow.h"

class TreeKeyPress : public QObject
{
    Q_OBJECT

protected:
    bool eventFilter(QObject *obj, QEvent *event, MainWindow *mw);
};

#endif // TREEKEYPRESS_H
