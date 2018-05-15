#ifndef THREADTABHELPER_H
#define THREADTABHELPER_H

#include "threadform.h"
#include "chans.h"
#include "filter.h"
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QtConcurrent/QtConcurrent>
#include <QImage>
#include <QSettings>

class ThreadTabHelper : public QObject
{
	Q_OBJECT
	bool gettingReply = false;
	Filter filterMe = filter;
public:
	ThreadTabHelper();
	~ThreadTabHelper();
	QString board;
	QString thread;
	QMap<QString,ThreadForm*> tfMap;
	QFutureWatcher<QImage> *imageScaler;
	bool abort = false;
	bool expandAll;
	static void writeJson(QString &board, QString &thread, QByteArray &rep);
	Chan *api;
	void getExtraFlags();
	QNetworkRequest request;
	QNetworkRequest requestFlags;
	QStringList allPosts;

private:
	QString threadUrl;
	QWidget *parent;
	bool isFromSession;
	QSet<QString> gottenFlags;

public slots:
	void startUp(Chan *api, QString board, QString thread, QWidget *parent, bool isFromSession);
	void loadPosts(QByteArray &posts, bool writeIt = true);
	void getPostsFinished();
	void loadExtraFlags();
	void filterTest(Post p);
	void reloadFilters();

signals:
	void postsLoaded(QJsonArray &posts);
	//void newTF(ThreadForm *tf);
	void newTF(Post p, ThreadFormStrings strings, bool loadFile = false);
	void windowTitle(QString windowTitle);
	void tabTitle(QString tabTitle);
	void setReplies(ThreadForm *tf);
	//void addStretch();
	void threadStatus(QString status, QString value = QString());
	void refresh(QPointer<ThreadForm> tf);
	void addNotification(QString p);
	void addReply(QString orig, QString no, bool isYou);
	void getPosts();
	void getFlags(QByteArray data);
	void setRegion(QString post_nr, QString region);
	void filterTested(QString no, bool filtered);
	void startFilterTest();

	//void scrollIt();
};

#endif // THREADTABHELPER_H
