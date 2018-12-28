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
#include <QPointer>
#include <QTimer>
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
protected:
	bool eventFilter(QObject *obj, QEvent *event);
public:
	explicit ThreadTab(Chan *api, QString &board, QString &thread, QWidget *parent = 0, bool isFromSession = false);
	~ThreadTab();

	Chan *api;
	QString board;
	QString thread;
	QMap<QString,ThreadForm*> tfMap;
	void findText(const QString &text);
	void loadAllImages();
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
	QString status;
	QScrollBar *vsb;
	bool atBottom = false;

public slots:
	//void addStretch();
	void focusIt();
	void focusMain();
	void quoteIt(QString text);
	void floatReply(const QString &link, int replyLevel = 0);
	void deleteFloat();
	void updateFloat();
	//void onNewTF(ThreadForm *tf);
	void onNewTF(Post p, ThreadFormStrings strings, bool loadFile = false);
	void onWindowTitle(QString title);
	void setFontSize(int fontSize);
	void setImageSize(int imageSize);
	void onAddReply(QString orig, QString no, bool isYou);
	void onAddNotification(QString no);
	void setAutoUpdate(bool update);
	void onThreadStatus(QString status,QString value = QString());
	void onGetFlags(QByteArray data);
	void getPosts();
	void onSetRegion(QString post_nr, QString region);
	void onFilterTest(QString no, bool filtered);
	bool vsbAtMax();

	//void checkScroll();

private:
	bool isFromSession;
	Ui::ThreadTab *ui;
	QMetaObject::Connection connectionVisibleChecker;
	void setShortcuts();
	QFuture< QList<ThreadForm*> > newImage;
	QFutureWatcher< QList<ThreadForm*> > watcher;
	QString vimCommand;
	QNetworkReply *postsReply;
	QNetworkReply *flagsReply;
	QTimer updateTimer;

private slots:
	void gallery();
	void openPostForm();
	void on_pushButton_clicked();
	void on_lineEdit_returnPressed();
	void removeTF(ThreadForm *tf);
	void showTF(ThreadForm *tf);
	void updateVim();
	void setTabTitle(QString tabTitle);
	void reloadFilters();


signals:
	void autoUpdate(bool update);
	void unseen(int totalUnseen);
	void formSeen();
	void startHelper(Chan *api, QString board, QString thread, QWidget *parent, bool isFromSession = false);
	void testFilters(Post p);

};

Q_DECLARE_METATYPE(ThreadTab*)

#endif // THREADTAB_H
