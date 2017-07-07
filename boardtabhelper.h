#ifndef BOARDTABHELPER_H
#define BOARDTABHELPER_H

#include "threadform.h"

enum BoardType{Index,Catalog};

class BoardTabHelper : public QObject
{
	Q_OBJECT
	bool gettingReply = false;
	Qt::ConnectionType UniqueDirect = static_cast<Qt::ConnectionType>(Qt::DirectConnection | Qt::UniqueConnection);
public:
	BoardTabHelper();
	~BoardTabHelper();
	QString board;
	BoardType type;
	QString search;
	QMap<QString,ThreadForm*> tfMap;
	QFutureWatcher<QImage> *imageScaler;
	bool abort = false;
	bool expandAll;
	void startUp(QString &board, BoardType type, QString search, QWidget *parent);
	static void writeJson(QString &board, QByteArray &rep);

private:
	QString boardUrl;
	QPointer<QNetworkReply> reply;
	QNetworkRequest request;
	QWidget *parent;
	QMetaObject::Connection connectionPost;
	QThread *updateThread;
	QMetaObject::Connection connectionUpdate;
	QJsonArray filterThreads(QByteArray &rep);

public slots:
	void loadPosts();
	void getPosts();
	void loadAllImages();
	void setAutoUpdate(bool update);

signals:
	void postsLoaded(QJsonArray &posts);
	void newThread(ThreadForm *tf);
	void newTF(ThreadForm *tf, ThreadForm *parent);
	void windowTitle(QString windowTitle);
	void setReplies(ThreadForm *tf);
	void addStretch();
	void boardStatus(QString status, QString value = "0");
	void refresh(QPointer<ThreadForm> tf);
	void clearMap();
	//void scrollIt();
};

#endif // BOARDTABHELPER_H
