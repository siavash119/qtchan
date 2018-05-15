#ifndef BOARDTAB_H
#define BOARDTAB_H

#include "boardtabhelper.h"
#include "chans.h"
#include "threadform.h"
#include "postform.h"
#include "filter.h"
#include <QSpacerItem>
#include <QWidget>
#include <QThread>

namespace Ui {
class BoardTab;
}

class BoardTab : public QWidget
{
	Q_OBJECT
public:
	explicit BoardTab(Chan *api, QString board, BoardType type = Index, QString search = "", QWidget *parent = 0);
	~BoardTab();
	Chan *api;
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
	void getPosts();
	void focusIt();
	ThreadForm* tfAtTop();

public slots:
	void findText(const QString text);
	void onNewThread(ThreadForm *tf);
	void onNewTF(ThreadForm *tf, ThreadForm *thread);
	//void addStretch();
	void clearMap();
	void setFontSize(int fontSize);
	void setImageSize(int imageSize);
	void removeTF(ThreadForm *tf);
	void showTF(ThreadForm *tf);

private:
	Ui::BoardTab *ui;
	QString vimCommand;
	void setShortcuts();

private slots:
	void on_pushButton_clicked();
	void on_lineEdit_returnPressed();
	void updateVim();
signals:
	void startHelper(Chan *api, QString &board, BoardType type, QString search, QWidget *parent);
};

Q_DECLARE_METATYPE(BoardTab*)

#endif // BOARDTAB_H
