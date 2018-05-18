#ifndef BOARDTABHELPER_H
#define BOARDTABHELPER_H

#include "chans.h"
#include "filter.h"
#include "threadform.h"

enum BoardType{Index,Catalog};

class BoardTabHelper : public QObject
{
	Q_OBJECT
	bool gettingReply = false;
	Qt::ConnectionType UniqueDirect = static_cast<Qt::ConnectionType>(Qt::DirectConnection | Qt::UniqueConnection);
	Filter filterMe = filter;
public:
	BoardTabHelper();
	~BoardTabHelper();
	QString board;
	BoardType type;
	QString search;
	QMap<QString,ThreadForm*> tfMap;
	bool abort = false;
	bool expandAll;
	static void writeJson(QString &path, QByteArray &rep);
	Chan *api;
	QStringList allPosts;
	QNetworkRequest request;
	QString title;

private:
	QString boardUrl;
	QWidget *parent;
	QMetaObject::Connection connectionPost;
	QMetaObject::Connection connectionUpdate;
	QJsonArray filterThreads(QByteArray &rep);
	PostKeys postKeys;
	QString filesPath;

public slots:
	void getPostsFinished();
	void setAutoUpdate(bool update);
	void reloadFilters();
	void filterTest(Post p);
	void startUp(Chan *api, QString board, BoardType type, QString search, QWidget *parent);

signals:
	void getPosts();
	void postsLoaded(QJsonArray &posts);
	//void newThread(ThreadForm *tf);
	void newReply(Post p, ThreadFormStrings strings, QString parentNum, bool loadFile = false);
	void newThread(Post p, ThreadFormStrings strings, bool loadFile = false);
	//void newTF(ThreadForm *tf, ThreadForm *parent);
	void windowTitle(QString windowTitle);
	void setReplies(ThreadForm *tf);
	void addStretch();
	void boardStatus(QString status, QString value = "0");
	void refresh(QPointer<ThreadForm> tf);
	void clearMap();
	void removeTF(ThreadForm *tf);
	void showTF(ThreadForm *tf);
	void filterTested(QString no, bool filtered);
	void startFilterTest();

	//void scrollIt();
};

#endif // BOARDTABHELPER_H
