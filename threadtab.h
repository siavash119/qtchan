#ifndef THREADTAB_H
#define THREADTAB_H

#include "postform.h"
#include "threadtabhelper.h"
#include "clickablelabel.h"
#include "treeitem.h"
#include <QWidget>
#include <QMutableMapIterator>
#include <QPointer>
#include <QThread>
#include <QSpacerItem>
#include <QFutureWatcher>
#include <QtConcurrent/QtConcurrent>

namespace Ui {
class ThreadTab;
}

class ThreadTab : public QWidget
{
	Q_OBJECT
	Qt::ConnectionType UniqueDirect = static_cast<Qt::ConnectionType>(Qt::DirectConnection | Qt::UniqueConnection);
	//QSpacerItem space = QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding);
protected:
	bool eventFilter(QObject *obj, QEvent *event);
public:
	explicit ThreadTab(QString board, QString thread, QWidget *parent = 0);
	~ThreadTab();

	QString board;
	QString thread;
	QString threadUrl;
	QMap<QString,ThreadForm*> tfMap;
	QList<ThreadForm*> unseenList;
	bool updated = false;
	void findText(const QString text);
	void loadAllImages();
	ThreadForm *findPost(QString postNum);
	int getMinWidth();
	void setMinWidth(int minw);
	bool floatIt;
	QPointer<ThreadForm> floating;
	PostForm myPostForm;
	QThread workerThread;
	ThreadTabHelper helper;
	int formsTotal = 0;
	int formsUnseen = 0;
	static QList<ThreadForm*> checkIfVisible(QList<ThreadForm*> &unseenList);
	TreeItem *tn;


public slots:
	//void addStretch();
	void focusIt();
	void updateWidth();
	void quoteIt(QString text);
	void floatReply(const QString &link, int replyLevel = 0);
	void deleteFloat();
	void updateFloat();
	void onNewTF(ThreadForm *tf);
	void onWindowTitle(QString title);
	//void checkScroll();

private:
	Ui::ThreadTab *ui;
	QMetaObject::Connection connectionAutoUpdate;
	QMetaObject::Connection connectionVisibleChecker;
	void setShortcuts();
	QFuture<QList<ThreadForm*>> newImage;
	QFutureWatcher<QList<ThreadForm*>> watcher;

private slots:
	void gallery();
	void openPostForm();
	void on_pushButton_clicked();
	void on_lineEdit_returnPressed();
	void removeTF(ThreadForm *tf);

signals:
	void autoUpdate(bool update);
	void unseen(int totalUnseen);
	void formSeen();
};

Q_DECLARE_METATYPE(ThreadTab*)

#endif // THREADTAB_H
