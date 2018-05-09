#ifndef THREADTABHELPER_H
#define THREADTABHELPER_H

#include "threadform.h"
#include "chans.h"
#include "filter.h"
#include <QNetworkReply>
#include <QtConcurrent/QtConcurrent>
#include <QImage>
#include <QSettings>

class ThreadTabHelper : public QObject
{
	Q_OBJECT
	bool gettingReply = false;
	Qt::ConnectionType UniqueDirect = static_cast<Qt::ConnectionType>(Qt::DirectConnection | Qt::UniqueConnection);
	QString colorString = "class=\\\"quote\\\" style=\\\"color:#8ba446\\\"";
	QString quoteString = "class=\\\"quote\\\" style=\\\"color:#897399\\\"";
	QRegExp quotesRegExp;
	QRegExp quotelinksRegExp;
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
	void startUp(Chan *api, QString &board, QString &thread, QWidget *parent, bool isFromSession);
	static void writeJson(QString &board, QString &thread, QByteArray &rep);
	Chan *api;
	void getExtraFlags();

private:
	QString threadUrl;
	QPointer<QNetworkReply> reply;
	QNetworkRequest request;
	QNetworkRequest requestFlags;
	QPointer<QNetworkReply> replyFlags;
	QWidget *parent;
	QMetaObject::Connection connectionPost;
	QTimer *updateTimer;
	QMetaObject::Connection connectionUpdate;
	bool isFromSession;
	QSet<QString> gottenFlags;

public slots:
	void loadPosts(QByteArray &posts, bool writeIt = true);
	void getPosts();
	void loadAllImages();
	void setAutoUpdate(bool update);
	void reloadFilters();

private slots:
	void getPostsFinished();
	void loadExtraFlags();

signals:
	void postsLoaded(QJsonArray &posts);
	void newTF(ThreadForm *tf);
	void windowTitle(QString windowTitle);
	void tabTitle(QString tabTitle);
	void setReplies(ThreadForm *tf);
	//void addStretch();
	void threadStatus(QString status, QString value = "0");
	void refresh(QPointer<ThreadForm> tf);
	void removeTF(ThreadForm *tf);
	void showTF(ThreadForm *tf);
	//void scrollIt();
};

#endif // THREADTABHELPER_H
