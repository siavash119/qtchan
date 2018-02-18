#ifndef NOTIFICATIONTRAY_H
#define NOTIFICATIONTRAY_H

#include "notificationview.h"
#include <QSystemTrayIcon>

class NotificationTray : public QSystemTrayIcon
{
	Q_OBJECT
public:
	NotificationView view;
	NotificationTray(QObject *parent = Q_NULLPTR);
	NotificationTray(const QIcon &icon, QObject *parent = Q_NULLPTR);
public slots:
	void activate(QSystemTrayIcon::ActivationReason reason);
};


#endif // NOTIFICATIONTRAY_H
