#include "postform.h"
#include "ui_postform.h"
#include "netcontroller.h"
#include <QUrl>
#include <QUrlQuery>
#include <QHttpMultiPart>
#include <QTimer>
#include <QFileDialog>
#include <QRegularExpression>
#include <QGraphicsItem>
#include <QShortcut>
#include <iostream>
#include <QGraphicsEffect>
#include <QRegularExpressionMatch>
#include <QSettings>

PostForm::PostForm(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::PostForm)
{
	ui->setupUi(this);
	ui->cancel->hide();
	QSettings settings;
	if(settings.value("use4chanPass", false).toBool() == true){
		ui->captcha->hide();
	}
	else {
		ui->challenge->hide();
	}
	this->setObjectName("PostForm");
	ui->name->installEventFilter(this);
	ui->email->installEventFilter(this);
	ui->subject->installEventFilter(this);
	ui->com->installEventFilter(this);
	ui->browse->installEventFilter(this);
	ui->response->installEventFilter(this);
	submitConnection = connect(ui->submit,&QPushButton::clicked,this,&PostForm::postIt);
	setShortcuts();
	connect(&captcha,&Captcha::challengeInfo,this,&PostForm::loadCaptchaImage);
	connect(ui->challenge,&ClickableLabel::clicked,[=]{
		captcha.getCaptcha();
	});
}

void PostForm::loadCaptchaImage(QString &challenege, QPixmap &challengeImage){
	(void)challenege;
	ui->challenge->show();
	qDebug() << "setting captcha image";
	ui->challenge->setPixmap(challengeImage);
}

void PostForm::load(QString &board, QString thread)
{
	this->board = board;
	this->thread = thread;
	this->setWindowTitle("post to /" + board + "/" + thread);
	ui->com->setFocus();
}

void PostForm::usePass(bool use4chanPass){
	if(use4chanPass){
		ui->captcha->hide();
	}
	else {
		ui->captcha->show();
		ui->challenge->hide();
	}
}

PostForm::~PostForm()
{
	delete ui;
}

void PostForm::setShortcuts()
{
	//override application shortcuts
	//new QShortcut(QKeySequence::NextChild,this);
	//new QShortcut(QKeySequence("Ctrl+Shift+Tab"),this);
	new QShortcut(QKeySequence::Delete,this);
	new QShortcut(Qt::Key_R,this);

	//rest in the eventfilter
}

void PostForm::appendText(QString &text)
{
	ui->com->textCursor().insertText(text);
}

void PostForm::postIt()
{
	addOverlay();
	disconnect(submitConnection);
	this->removeEventFilter(this);
	qDebug().noquote() << "posting";
	QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

	QHttpPart mode;
	mode.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"mode\""));
	mode.setBody("regist");
	multiPart->append(mode);

	if(thread != ""){
		QHttpPart resto;
		resto.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"resto\""));
		resto.setBody(thread.toStdString().c_str());
		multiPart->append(resto);
	}

	QHttpPart name;
	name.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"name\""));
	name.setBody(ui->name->text().toStdString().c_str());
	multiPart->append(name);

	QHttpPart email;
	email.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"email\""));
	email.setBody(ui->email->text().toStdString().c_str());
	multiPart->append(email);

	QHttpPart com;
	com.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"com\""));
	qDebug().noquote() << ui->com->toPlainText().toStdString().c_str();
	com.setBody(ui->com->toPlainText().toStdString().c_str());
	multiPart->append(com);

	QSettings settings;
	if(settings.value("use4chanPass", false).toBool() == false){
		QHttpPart captchaChallenge;
		captchaChallenge.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"recaptcha_challenge_field\""));
		if(captcha.challenge.isEmpty()){
			multiPart->deleteLater();
			return;
		}
		qDebug().noquote() << captcha.challenge.toStdString().c_str();
		captchaChallenge.setBody(captcha.challenge.toStdString().c_str());

		QHttpPart captchaResponse;
		captchaResponse.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"recaptcha_response_field\""));
		if(ui->response->text().isEmpty()){
			multiPart->deleteLater();
			return;
		}
		captchaResponse.setBody(ui->response->text().toStdString().c_str());

		multiPart->append(captchaChallenge);
		multiPart->append(captchaResponse);
	}

	if(filename != "") {
		QHttpPart uploadFile;
		uploadFile.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/octet-stream"));
		uploadFile.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"upfile\"; filename=\""+filename+"\""));
		QFile *upQFile = new QFile(filename);
		upQFile->open(QIODevice::ReadOnly);
		uploadFile.setBodyDevice(upQFile);
		upQFile->setParent(multiPart);
		multiPart->append(uploadFile);
	}

	QUrl url = QUrl("https://sys.4chan.org/"+board+"/post");
	QNetworkRequest request(url);
	//request.1
	postReply = nc.jsonManager->post(request, multiPart);
	connect(postReply, &QNetworkReply::finished, this, &PostForm::postFinished);
	multiPart->setParent(postReply); // delete the multiPart with the reply
	// here connect signals etc.
}

void PostForm::postFinished()
{
	QTextEdit *reply = new QTextEdit();
	reply->setAttribute(Qt::WA_DeleteOnClose);
	connect(reply,&QTextEdit::destroyed,[=]{
		qDebug() << "response window destroyed";
	});
	reply->setWindowTitle("post to /"+board+"/"+thread+" response");
	reply->setMinimumSize(800,600);
	QByteArray temp = postReply->readAll();
	reply->setHtml(temp);
	reply->setGeometry(0,0,this->width(),this->height());
	qDebug().noquote() << temp;
	qDebug() << "showing reply";
	QString replyString(reply->toPlainText());
	if(replyString.contains(QRegularExpression("uploaded.$|Post successful!$")))
	{
		overlay->displayText = replyString;
		ui->com->clear();
		setFilenameText(empty);
		filename = "";
		ui->cancel->hide();
		QTimer::singleShot(1000, this, &PostForm::close);
		if(thread == ""){
			//QRegularExpression re("<h1.*?>(?<upMessage>.*?)</h1><!-- thread:0,no:(?<threadNum>\\d+)");
			QRegularExpression re("<!-- thread:0,no:(?<threadNum>\\d+)");
			QRegularExpressionMatch match = re.match(temp);
			if(match.hasMatch()){
				qDebug() << "post successful; loading thread:" << match.captured("threadNum");
				emit loadThread(match.captured("threadNum"));
			}
			else{
				qDebug() << "post succesful; but some other error";
				qDebug() << replyString;
			}
		}
		//QTimer::singleShot(1000, reply, &QTextEdit::close);
	}
	else{
		reply->show();
	}
	QTimer::singleShot(1000, this,&PostForm::removeOverlay);
	captcha.loaded = false;
	this->installEventFilter(this);
	submitConnection = connect(ui->submit,&QPushButton::clicked,this,&PostForm::postIt);
}

void PostForm::addOverlay()
{
	qDebug().noquote() << "adding overlay";
	overlay = new Overlay(this);
	overlay->show();
	focused = this->focusWidget();
	ui->com->setFocusPolicy(Qt::NoFocus);
	ui->filename->setFocus();
}

void PostForm::removeOverlay()
{
	qDebug().noquote() << "removing overlay";
	delete overlay;
	ui->com->setFocusPolicy(Qt::StrongFocus);
	focused->setFocus();
}

bool PostForm::eventFilter(QObject *obj, QEvent *event)
{
	if (event->type() == QEvent::KeyPress) {
		QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
		int mod = keyEvent->modifiers();
		int key = keyEvent->key();
		//qDebug("Ate key press %d", key);
		//qDebug("Ate modifier press %d", mod);
		//escape to get out
		if(key == 16777216) {
			hide();
		}
		//shift+enter to post
		if(mod == 33554432 && key == 16777220) {
			postIt();
			return true;
		}
		return QObject::eventFilter(obj, event);
	} else if(event->type() == QEvent::DragEnter) {
		static_cast<QDragEnterEvent*>(event)->acceptProposedAction();
		return false;
	} else if(event->type() == QEvent::Drop) {
		const QMimeData *mimeData = static_cast<QDropEvent*>(event)->mimeData();
		fileChecker(mimeData);
		qDebug().noquote() << "DROPPED";
		return false;
	}
	if(obj->objectName() == "response"){
		if(event->type() == QEvent::FocusIn){
			if(!captcha.loaded) captcha.getCaptcha();
		}
	}
	return QObject::eventFilter(obj, event);
}

void PostForm::fileChecker(const QMimeData *mimeData)
{
	QString output;
	qDebug().noquote() << "checking file";
	if (mimeData->hasImage()) {
		//ui->label->setPixmap(qvariant_cast<QPixmap>(mimeData->imageData()));
	} else if (mimeData->hasHtml()) {
		output = QUrl::fromPercentEncoding(mimeData->text().toUtf8());
		setFilenameText(output);
	} else if (mimeData->hasText()) {
		qDebug().noquote() << mimeData->text();
		output = QUrl::fromPercentEncoding(mimeData->text().toUtf8());
#if defined(Q_OS_WIN)
		filename = output.mid(8); //remove file:///
#else
		filename = output.mid(7); //remove file://
#endif
		filename.remove(QRegularExpression("[\\n\\t\\r]"));
		qDebug() << "added file to upload:" << filename;
		setFilenameText(filename);
	} else if (mimeData->hasUrls()) {
		QList<QUrl> urlList = mimeData->urls();
		QString text;
		for (int i = 0; i < urlList.size() && i < 32; ++i)
			text += urlList.at(i).path() + QLatin1Char('\n');
		output = QUrl::fromPercentEncoding(text.toUtf8());
		setFilenameText(output);
	} else {
		QString temp("Cannot display data");
		setFilenameText(temp);
	}
	qDebug().noquote() << filename;
	ui->cancel->show();
}


void PostForm::on_browse_clicked()
{
	dialog = new QFileDialog(this);
	dialog->setFileMode(QFileDialog::AnyFile);
	dialog->show();
	connect(dialog,&QFileDialog::fileSelected,this,&PostForm::fileSelected);
}

void PostForm::fileSelected(const QString &file)
{
	filename = file;
	//qDebug() << dialog->;
	//qDebug() << dialog->getOpenFileName();
	if(filename == "") {
		setFilenameText(empty);
		ui->cancel->hide();
	}
	else {
		setFilenameText(filename);
		ui->cancel->show();
	}
	dialog->close();
	qDebug().noquote() << filename;
}

void PostForm::on_cancel_clicked()
{
	filename = "";
	setFilenameText(empty);
	ui->cancel->hide();
}

void PostForm::droppedItem()
{
	qDebug().noquote() << "dropped!";
}

void PostForm::setFilenameText(QString &text){
	QFontMetrics metrics(ui->filename->font());
	QString elidedText = metrics.elidedText(text, Qt::ElideRight, ui->filename->width());
	ui->filename->setText(elidedText);
}

void PostForm::resizeEvent(QResizeEvent *event){
	if(filename != "") setFilenameText(filename);
	else setFilenameText(empty);
	return QWidget::resizeEvent(event);
}
