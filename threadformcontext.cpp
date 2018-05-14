#include "threadformcontext.h"

ThreadFormContext::ThreadFormContext(Post *p, QWidget *parent) :
	p(p)
{
	setParent(parent);
	setAttribute(Qt::WA_DeleteOnClose);
	setStyleSheet("background-color:#222222;color:#bbbbbb;border:1px solid white");
	addAction(p->no);
	QMenu *filterMenu = addMenu("Filter");
	addFilter(filterMenu,"Number: ","no",p->no);
	addFilter(filterMenu,"Name: ","name",p->name);
	if(!p->sub.isEmpty()) addFilter(filterMenu,"Subject: ","sub",p->sub);
	if(!p->trip.isEmpty()) addFilter(filterMenu,"Trip: ","trip",p->trip);
	if(!p->md5.isEmpty()) addFilter(filterMenu,"MD5: ","md5",p->md5);
	popup(QCursor::pos());
}

void ThreadFormContext::addFilter(QMenu *menu, QString name, QString key, QString value){
	QAction *newFilter = menu->addAction(name % value);
	connect(newFilter,&QAction::triggered,[=]{
		QString exp = value;
		filter.addFilter2(key,Filter::filterEscape(exp),"boards:"+p->board);
		emit filtersChanged();
	});
}
