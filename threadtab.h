#ifndef THREADTAB_H
#define THREADTAB_H

#include "postform.h"
#include "threadtabhelper.h"
#include "clickablelabel.h"
#include "threadinfo.h"
#include "treeitem.h"
#include "filter.h"
#include "chans.h"
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
	explicit ThreadTab(Chan *api, QString board, QString thread, QWidget *parent = 0, bool isFromSession = false);
	~ThreadTab();

	Chan *api;
	QString board;
	QString thread;
	QString threadUrl;
	QMap<QString,ThreadForm*> tfMap;
	bool updated = false;
	void findText(const QString &text);
	void loadAllImages();
	ThreadForm *findPost(QString postNum);
	bool floatIt;
	QPointer<ThreadForm> floating;
	PostForm myPostForm;
	QThread workerThread;
	ThreadTabHelper helper;
	static QList<ThreadForm*> checkIfVisible(QList<ThreadForm*> &unseenList);
	TreeItem *tn;

	ThreadInfo info;
	QList<ThreadForm*> unseenList;
	int formsTotal = 0;
	int formsUnseen = 0;
	ThreadForm* tfAtTop();
	ThreadForm* tfAtBottom();

public slots:
	//void addStretch();
	void focusIt();
	void quoteIt(QString text);
	void floatReply(const QString &link, int replyLevel = 0);
	void deleteFloat();
	void updateFloat();
	void onNewTF(ThreadForm *tf);
	void onWindowTitle(QString title);
	void setFontSize(int fontSize);
	void setImageSize(int imageSize);
	//void checkScroll();

private:
	bool isFromSession;
	Ui::ThreadTab *ui;
	QMetaObject::Connection connectionAutoUpdate;
	QMetaObject::Connection connectionVisibleChecker;
	void setShortcuts();
	QFuture<QList<ThreadForm*>> newImage;
	QFutureWatcher<QList<ThreadForm*>> watcher;
	Filter filter;
	QString vimCommand;

private slots:
	void gallery();
	void openPostForm();
	void on_pushButton_clicked();
	void on_lineEdit_returnPressed();
	void removeTF(ThreadForm *tf);
	void updateVim();

signals:
	void autoUpdate(bool update);
	void unseen(int totalUnseen);
	void formSeen();
};

Q_DECLARE_METATYPE(ThreadTab*)

#endif // THREADTAB_H
