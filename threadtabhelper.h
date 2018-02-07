#ifndef THREADTABHELPER_H
#define THREADTABHELPER_H

#include "threadform.h"
#include "chans.h"
#include <QNetworkReply>
#include <QtConcurrent/QtConcurrent>
#include <QFutureWatcher>
#include <QImage>
#include <QMetaObject>
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

private:
	QString threadUrl;
	QPointer<QNetworkReply> reply;
	QNetworkRequest request;
	QWidget *parent;
	QMetaObject::Connection connectionPost;
	QPointer<QTimer> updateTimer;
	QThread *updateThread;
	QMetaObject::Connection connectionUpdate;
	bool isFromSession;

public slots:
	void loadPosts();
	void getPosts();
	void loadAllImages();
	void setAutoUpdate(bool update);

signals:
	void postsLoaded(QJsonArray &posts);
	void newTF(ThreadForm *tf);
	void windowTitle(QString windowTitle);
	void tabTitle(QString tabTitle);
	void setReplies(ThreadForm *tf);
	//void addStretch();
	void threadStatus(QString status, QString value = "0");
	void refresh(QPointer<ThreadForm> tf);
	//void scrollIt();
};

#endif // THREADTABHELPER_H
