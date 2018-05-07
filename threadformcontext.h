#ifndef THREADFORMCONTEXT_H
#define THREADFORMCONTEXT_H

#include "post.h"
#include "filter.h"
#include <QMenu>

class ThreadFormContext : public QMenu
{
	Q_OBJECT
public:
	explicit ThreadFormContext(Post *p, QWidget* parent = Q_NULLPTR);
	Post *p;
	void addFilter(QMenu *menu, QString name, QString key, QString value);
private:

signals:
	void filtersChanged();
};

#endif // THREADFORMCONTEXT_H
