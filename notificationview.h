#ifndef NOTIFICATIONVIEW_H
#define NOTIFICATIONVIEW_H

#include <QFrame>

namespace Ui {
class NotificationView;
}

class NotificationView : public QFrame
{
	Q_OBJECT

public:
	explicit NotificationView(QWidget *parent = 0);
	~NotificationView();
	QPoint toMove();
private:
	QList<QMetaObject::Connection> allConnections;
public slots:
	void addNotification(QWidget *note);
	void reAdjust();
private slots:
	void on_clear_clicked();
	void on_close_clicked();

private:
	Ui::NotificationView *ui;
};

extern NotificationView *nv;

#endif // NOTIFICATIONVIEW_H
