#ifndef POSTFORM_H
#define POSTFORM_H

#include "overlay.h"
#include "captcha.h"
#include "chans.h"
#include <QFontMetrics>
#include <QWidget>
#include <QNetworkReply>
#include <QFileDialog>
#include <QMimeData>
#include <QTextEdit>

namespace Ui {
class PostForm;
}

class PostForm : public QWidget
{
	Q_OBJECT
public:
	explicit PostForm(QWidget *parent = 0);
	void load(Chan *api, QString &board, QString thread = "0");
	~PostForm();
	void postIt();
	QString board;
	QString thread;
	QString filename;
	QNetworkReply *postReply;
	QNetworkReply *captchaReply;
	QFileDialog *dialog;
	QMetaObject::Connection submitConnection;
	void fileChecker(const QMimeData *mimedata);
	void addOverlay();
	void removeOverlay();
	QString empty = "No file selected";
	void verifyCaptcha();
	//TODO links in com?

signals:
	void loadThread(QString thread);
	void addYou(QString postNum);

public slots:
	void appendText(QString &text);
	void loadCaptchaImage(QString &challenege, QPixmap &challengeImage);
	void usePass(bool use4chanPass);
	void setFontSize(int fontSize);
	void loadCaptchaQuestion(QString &challenge);

private:
	Ui::PostForm *ui;
	bool isPosting;
	void cancelPost();
	Overlay *overlay;
	QWidget *focused;
	Captcha captcha;
	void setFilenameText(QString &text);
	Chan *api;
	QString captchaCode;

private slots:
	void postFinished();
	void fileSelected(const QString &file);
	void on_browse_clicked();
	void on_cancel_clicked();
	void droppedItem();
	void setShortcuts();

	void on_response_returnPressed();

protected:
	bool eventFilter(QObject *obj, QEvent *event);
	void resizeEvent(QResizeEvent *event);
};

#endif // POSTFORM_H
