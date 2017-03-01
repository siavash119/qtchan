#include "treekeypress.h"
#include <QEvent>
#include <QKeyEvent>

bool TreeKeyPress::eventFilter(QObject *obj, QEvent *event, MainWindow *mw)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        qDebug("Ate key press %d", keyEvent->key());
        if(keyEvent->key() == 16777223){
            qDebug() << "deleting";
            mw->deleteSelected();
        }
        return true;
    } else {
        // standard event processing
        return QObject::eventFilter(obj, event);
    }
}
