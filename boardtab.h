#ifndef BOARDTAB_H
#define BOARDTAB_H

#include "boardtabhelper.h"
#include "threadform.h"
#include "postform.h"
#include <QSpacerItem>
#include <QWidget>
#include <QThread>

namespace Ui {
class BoardTab;
}

class BoardTab : public QWidget
{
	Q_OBJECT
	Qt::ConnectionType UniqueDirect = static_cast<Qt::ConnectionType>(Qt::DirectConnection | Qt::UniqueConnection);
	//QSpacerItem space = QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding);
public:
	explicit BoardTab(QString board, BoardType type = BoardType::Index, QString search = "", QWidget *parent = 0);
	~BoardTab();

	QString board;
	BoardType type;
	QString search;
	QString tabType;
	QThread workerThread;
	BoardTabHelper helper;
	QString boardUrl;
	QNetworkReply *reply;
	QMap<QString,ThreadForm*> tfMap;
	PostForm myPostForm;
	void openPostForm();

	void setShortcuts();
	void getPosts();
	void focusIt();

public slots:
	void findText(const QString text);
	void onNewThread(ThreadForm *tf);
	void onNewTF(ThreadForm *tf, ThreadForm *thread);
	//void addStretch();
	void clearMap();
	void setFontSize(int fontSize, int imageSize);

private:
	Ui::BoardTab *ui;

private slots:
	void on_pushButton_clicked();
	void on_lineEdit_returnPressed();
};

Q_DECLARE_METATYPE(BoardTab*)

#endif // BOARDTAB_H
