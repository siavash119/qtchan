#include "notificationtray.h"
#include <QDebug>

NotificationTray::NotificationTray(QObject *parent):
	QSystemTrayIcon(parent)
{
	connect(this,&QSystemTrayIcon::activated,this,&NotificationTray::activate);
}

NotificationTray::NotificationTray(const QIcon &icon, QObject *parent):
	QSystemTrayIcon(icon,parent)
{
	connect(this,&QSystemTrayIcon::activated,this,&NotificationTray::activate);
}

void NotificationTray::activate(QSystemTrayIcon::ActivationReason reason){
	if(reason == QSystemTrayIcon::ActivationReason::Trigger || reason == QSystemTrayIcon::ActivationReason::DoubleClick){
		if(nv->isVisible()) nv->hide();
		else{
			nv->move(nv->toMove());
			nv->show();
		}
	}
}
