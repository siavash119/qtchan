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
	QMap<QString,ThreadForm*> tfReplyMap;
	PostForm myPostForm;

	void openPostForm();
	void getPosts();
	void focusIt();
	void focusMain();

public slots:
	void findText(const QString text);
	void onNewThread(Post post, ThreadFormStrings strings, bool loadFile = false);
	void onNewReply(Post post, ThreadFormStrings strings, QString opNum, bool loadFile = false);
	//void onNewThread(ThreadForm *tf);
	//void onNewReply(ThreadForm *tf, ThreadForm *thread);
	void clearMap();
	void setFontSize(int fontSize);
	void setImageSize(int imageSize);
	void removeTF(ThreadForm *tf);
	void showTF(ThreadForm *tf);
	void onFilterTest(QString no, bool filtered);
	void loadAllImages();


private:
	Ui::BoardTab *ui;
	QString vimCommand;
	void setShortcuts();
	QPointer<QNetworkReply> postsReply;
	ThreadForm* tfAtTop();

private slots:
	void on_pushButton_clicked();
	void on_lineEdit_returnPressed();
	void updateVim();
	void reloadFilters();

signals:
	void startHelper(Chan *api, QString board, BoardType type, QString search, QWidget *parent);
	void testFilters(Post p);

};

Q_DECLARE_METATYPE(BoardTab*)

#endif // BOARDTAB_H
