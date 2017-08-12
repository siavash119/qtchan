#ifndef THREADFORM_H
#define THREADFORM_H

#include "post.h"
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
	explicit ThreadForm(QString board, QString threadNum, PostType type = Reply,
						bool root = true, bool autoExpand = false, QWidget *parent = 0, int replyLevel = 0);
	~ThreadForm();
	void setText(QString text);
	void setImage(QByteArray img);
	//void getImage(QNetworkAccessManager *manager, QString *img);
	void load(QJsonObject &p);
	void loadImage(QString path);
	void openImage();
	QString board;
	QString threadNum;
	PostType type;
	bool root;
	bool autoExpand;
	static QString htmlParse(QString &html);
	static QString titleParse(QString &title);
	//Post *post;
	Post post;
	void updateComHeight();
	QDir *folder;
	QString folderPath;
	//void setImage(QString text);
	QSignalMapper *signalMapper;
	//void insert(int position, ThreadForm *tf);
	void insert(ThreadForm *tf);
	void deleteHideLayout();
	QSet<QString> quotelinks;
	QMap<double,QString> replies;
	void setReplies();
	void setRepliesString();
	void setInfoString();
	void loadOrig();
	ThreadForm *clone(int replyLevel = 0);
	QList<QPointer<ThreadForm>> clones;
	//TODO check settings -> filter
	bool hidden = false;
	bool seen = false;
	static QImage scaleImage(QString path, int scale);
	QFutureWatcher<QImage> watcher;
	QString repliesString;
	QString infoString();
	void addReplyLink(QString &reply);
	int replyLevel;
	bool hasImage = true;

private:
	QWidget *tab;
	QString lastLink;
	Ui::ThreadForm *ui;
	bool loadIt;
	QString fileURL;
	QString thumbURL;
	QString pathBase;
	QString filePath;
	QPointer<QFile> file;
	QString thumbPath;
	QPointer<QFile> thumb;
	QMetaObject::Connection connectionThumb;
	QMetaObject::Connection connectionImage;
	//QNetworkReply *reply; //Use for cross-thread gets later?
	QNetworkReply *replyThumb;
	QNetworkReply *replyImage;
	void getFile();
	void getThumb();
	bool finished = false;
	bool hideButtonShown = true;
	int darkness = 22;
	QColor background;
	void clickImage();

signals:
	void loadThreadTab(ThreadForm*, QJsonArray&);
	void loadThread(ThreadForm*,QString&,QString&);
	void updateWidth();
	void floatLink(const QString &link, int replyLevel = 0);
	void updateFloat();
	void removeMe(QPointer<ThreadForm> tf);
	void fileFinished();
	void deleteFloat();
	//void searchPost(int position, QString postNum);

public slots:
	void getOrigFinished();
	void getThumbFinished();
	void imageClicked();
	void hideClicked();
	void removeClone(QPointer<ThreadForm> tf);
	void addReply(ThreadForm *tf);
	void setFontSize(int fontSize, int imageSize);

private slots:
	void quoteClicked(const QString &link);
	void appendQuote();
	void alreadyClicked();
	//void downloading(qint64 read, qint64 total);

	void on_com_linkHovered(const QString &link);
	void on_info_linkHovered(const QString &link);

protected:
	bool eventFilter(QObject *obj, QEvent *event);
	void paintEvent(QPaintEvent *e);
	//void mouseMoveEvent(QMouseEvent *event);
};

#endif // THREADFORM_H
