#ifndef THREADFORM_H
#define THREADFORM_H

#include "post.h"
#include "chans.h"
#include "threadformstrings.h"
#include "clickablelabel.h"
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
	Qt::ConnectionType UniqueDirect = static_cast<Qt::ConnectionType>(Qt::DirectConnection | Qt::UniqueConnection);
public:
	explicit ThreadForm(Chan *api, ThreadFormStrings strings, bool root = true,
						bool autoExpand = false, QWidget *parent = 0, int replyLevel = 0);
	~ThreadForm();
	void setText(QString text);
	void setImage(QByteArray img);
	//void getImage(QNetworkAccessManager *manager, QString *img);
	void load(Post &post);
	void loadImage(int labelInd, QString path);
	void openImage(const QString &path);
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
	static QPair<int,QImage> scaleImage(int labelInd, QString path, int scale);
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
	void getFile(ClickableLabel *label, int ind, bool andOpen = false);
	QStringList regionList;
	void setRegion(const QString &region);
	QString fileInfoString;
	QString tfInfoString;
	QString matchThis();
	QString regionString;
	void getFiles();
	QList<bool> gettingFile;
	QList<bool> finished;
	void setPixmap(int ind, QPixmap scaled);
	void openThread();

//TODO take care of file downloading in netcontroller
private:
	QWidget *tab;
	ThreadForm *rootTF;
	Ui::ThreadForm *ui;
	QList<ClickableLabel*> labels;
	void getThumb();
	bool hideButtonShown = true;
	int darkness = 25;
	QColor background;
	void clickImage();
	bool loadIt = false;
	void downloadFile(const QString &fileUrl,const QString &filePath,QNetworkAccessManager *manager,
					  QString type = QString(), QString message = QString(), ClickableLabel *label = Q_NULLPTR);
	QHash<QString,QNetworkReply*> networkReplies;
	int regionsGot = 0;
	QMap<QString,ThreadForm*> inserted;
	QMap<QString,QMetaObject::Connection> insertedConnections;
	void removeFromInserted();
	void postMenu();
	void reloadFiles();

signals:
	void loadThreadTab(ThreadForm*, QJsonArray&);
	void loadThread(ThreadForm*,QString&,QString&);
	void updateWidth();
	void floatLink(const QString &link, int replyLevel = 0);
	void updateFloat();
	void removeMe(QPointer<ThreadForm> tf);
	void deleteFloat();
	//void searchPost(int position, QString postNum);

public slots:
	void imageClicked();
	void hideClicked();
	void removeClone(QPointer<ThreadForm> tf);
	void addReply(ThreadForm *tf);
	void setFontSize(int fontSize);
	void setImageSize(int imageSize);
	void setBackground();
	void downloadedSlot(const QString &path, const QString &type, const QString &message, ClickableLabel *label);

private slots:
	void quoteClicked(const QString &link);
	void appendQuote();
	//void downloading(qint64 read, qint64 total);

	void on_com_linkHovered(const QString &link);
	void on_info_linkHovered(const QString &link);
	void scaleFinished();

protected:
	bool eventFilter(QObject *obj, QEvent *event);
	//void mouseMoveEvent(QMouseEvent *event);
};

#endif // THREADFORM_H
