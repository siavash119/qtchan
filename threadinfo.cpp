#include "threadinfo.h"
#include "ui_threadinfo.h"

ThreadInfo::ThreadInfo(QWidget *parent) :
	QFrame(parent),
	ui(new Ui::ThreadInfo)
{
	ui->setupUi(this);
}

ThreadInfo::~ThreadInfo()
{
	delete ui;
}

void ThreadInfo::updateFields(){
	ui->posts->setText(QString::number(posts));
	ui->files->setText(QString::number(files));
}

void ThreadInfo::mousePressEvent(QMouseEvent *event)
{
	offset = event->pos();
}

void ThreadInfo::mouseMoveEvent(QMouseEvent *event)
{
	if(event->buttons() & Qt::LeftButton)
	{
		this->move(mapToParent(event->pos() - offset));
	}
}
