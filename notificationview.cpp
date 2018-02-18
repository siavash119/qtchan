#include "notificationview.h"
#include "ui_notificationview.h"
#include <QDesktopWidget>

NotificationView::NotificationView(QWidget *parent) :
	QFrame(parent),
	ui(new Ui::NotificationView)
{
	this->setWindowFlags(Qt::ToolTip);
	ui->setupUi(this);
}

NotificationView::~NotificationView()
{
	foreach(QMetaObject::Connection noteConnection, allConnections){
		disconnect(noteConnection);
	}
	delete ui;
}

void NotificationView::addNotification(QWidget *note){
	ui->posts->insertWidget(0,note);
	note->setParent(this);
	reAdjust();
	allConnections << connect(note,&QWidget::destroyed,this,&NotificationView::reAdjust);
}

void NotificationView::reAdjust(){
	adjustSize();
	move(toMove());
}

QPoint NotificationView::toMove(){
	QRect rec = QApplication::desktop()->availableGeometry(this);
	return QPoint(rec.width()-this->width()-32,rec.height()-this->height()-32);
}

void NotificationView::on_clear_clicked(){
	while (QLayoutItem* item = ui->posts->takeAt(0)){
	   if (QWidget* widget = item->widget()){
		   widget->deleteLater();
	   }
	}
}

void NotificationView::on_close_clicked(){
	hide();
}
