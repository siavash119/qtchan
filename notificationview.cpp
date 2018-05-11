#include "notificationview.h"
#include "ui_notificationview.h"
#include <QDesktopWidget>

NotificationView::NotificationView(QWidget *parent) :
	QFrame(parent),
	ui(new Ui::NotificationView)
{
	this->setWindowFlags(Qt::ToolTip);
	ui->setupUi(this);
	this->setMaximumHeight(QApplication::desktop()->availableGeometry().height()-64);
	reAdjust();
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
	reAdjust();
	allConnections << connect(note,&QWidget::destroyed,this,&NotificationView::reAdjust);
}

//TODO should be an easier way to do this
void NotificationView::reAdjust(){
	QRect rec = QApplication::desktop()->availableGeometry(this);
	int contentHint = ui->content->sizeHint().height();
	int scrollHint = ui->scrollArea->sizeHint().height();
	int recHint = rec.height()-64;
	int hint = ((contentHint < scrollHint) ? scrollHint : contentHint) + ui->buttons->sizeHint().height();
	int height = (hint < recHint) ? hint : recHint;
	height = (height < sizeHint().height()) ? sizeHint().height() : height;
	setGeometry(
				rec.width()-680-32,
				rec.height()-height-32,
				680,
				height);
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
