#ifndef THREADFORM_H
#define THREADFORM_H

#include "post.h"
#include "chans.h"
#include "threadformstrings.h"
#include <QFutureWatcher>
#include <QWidget>
#include <QPointer>
#include <QNetworkReply>
#include <QByteArray>
#include <QDir>
#include <QSet>
#include <QList>
#include <QSignalMapper>
#include <QMouseEvent>

namespace Ui {
class ThreadForm;
}

enum PostType { Thread, Reply };

class ThreadForm : public QWidget
{
	Q_OBJECT
	bool gettingFile = false;
	bool gettingThumb = false;
	Qt::ConnectionType UniqueDirect = static_cast<Qt::ConnectionType>(Qt::DirectConnection | Qt::UniqueConnection);
public:
	explicit ThreadForm(Chan *api, ThreadFormStrings strings, bool root = true,
						bool autoExpand = false, QWidget *parent = 0, int replyLevel = 0);
	~ThreadForm();
	void setText(QString text);
	void setImage(QByteArray img);
	//void getImage(QNetworkAccessManager *manager, QString *img);
	void load(Post &post);
	void loadImage(QString path);
	void openImage();
	Chan *api;
	ThreadFormStrings strings;
	PostType type;
	QString board;
	bool root;
	bool autoExpand;
	Post post;
	//void setImage(QString text);
	QSignalMapper *signalMapper;
	//void insert(int position, ThreadForm *tf);
	void insert(ThreadForm *tf);
	void deleteHideLayout();
	QMap<double,QString> replies;
	void setReplies();
	void setRepliesString(const QString &repliesString);
	ThreadForm *clone(int replyLevel = 0);
	QList< QPointer<ThreadForm> > clones;
	//TODO check settings -> filter
	bool hidden = false;
	bool seen = false;
	static QImage scaleImage(QString path, int scale);
	QString repliesString;
	QString infoString();
	QString getInfoString();
	void setInfoString();
	void addReplyLink(QString &reply, bool isYou = false);
	int replyLevel;
	bool hasImage = true;
	QMetaObject::Connection comQuoteConnection;
	QMetaObject::Connection infoQuoteConnection;
	void getFlag();
	void getFile(bool andOpen = false);
	QStringList regionList;
	void setRegion(const QString &region);
	QString fileInfoString;
	QString tfInfoString;
	QString matchThis();
	QString regionString;

//TODO take care of file downloading in netcontroller
private:
	QWidget *tab;
	ThreadForm *rootTF;
	Ui::ThreadForm *ui;
	QPointer<QFile> file;
	QPointer<QFile> thumb;
	void getThumb();
	bool finished = false;
	bool hideButtonShown = true;
	int darkness = 22;
	QColor background;
	void clickImage();
	bool loadIt = false;
	void downloadFile(const QString &fileUrl,const QString &filePath,QNetworkAccessManager *manager, QString message = QString());
	QHash<QString,QNetworkReply*> networkReplies;
	int regionsGot = 0;
	QMap<QString,ThreadForm*> inserted;
	QMap<QString,QMetaObject::Connection> insertedConnections;
	void removeFromInserted();
	void postMenu();
	QPixmap scaled;
	QFutureWatcher<QImage> watcher;

signals:
	void loadThreadTab(ThreadForm*, QJsonArray&);
	void loadThread(ThreadForm*,QString&,QString&);
	void updateWidth();
	void floatLink(const QString &link, int replyLevel = 0);
	void updateFloat();
	void removeMe(QPointer<ThreadForm> tf);
	void deleteFloat();
	void setPixmap();
	//void searchPost(int position, QString postNum);

public slots:
	void imageClicked();
	void hideClicked();
	void removeClone(QPointer<ThreadForm> tf);
	void addReply(ThreadForm *tf);
	void setFontSize(int fontSize);
	void setImageSize(int imageSize);
	void downloadedSlot(const QString &path, const QString &message);
	void onSetPixmap();

private slots:
	void quoteClicked(const QString &link);
	void appendQuote();
	//void downloading(qint64 read, qint64 total);

	void on_com_linkHovered(const QString &link);
	void on_info_linkHovered(const QString &link);

protected:
	bool eventFilter(QObject *obj, QEvent *event);
	void paintEvent(QPaintEvent *e);
	//void mouseMoveEvent(QMouseEvent *event);
};

#endif // THREADFORM_H
