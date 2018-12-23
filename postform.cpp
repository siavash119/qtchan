#include "postform.h"
#include "ui_postform.h"
#include "netcontroller.h"
#include "you.h"
#include <QUrl>
#include <QUrlQuery>
#include <QHttpMultiPart>
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
	ui(new Ui::PostForm),
	isPosting(false)
{
	ui->setupUi(this);
	ui->cancel->hide();
	QSettings settings;
	if(settings.value("use4chanPass", false).toBool() == true){
		ui->captcha->hide();
		captchaTimer = Q_NULLPTR;
	}
	else {
		ui->question->hide();
		ui->challenge->hide();
		ui->refreshCaptcha->hide();
		captchaTimer = new QTimer(this);
		captchaTimer->setSingleShot(true);
		connect(captchaTimer,&QTimer::timeout,[=]{
			ui->question->setText("Captcha code expired");
			ui->question->show();
			ui->refreshCaptcha->show();
			ui->response->show();
			captchaCode = "";
			captcha.loaded = false;
			captcha.loading = false;
			//captcha.getCaptcha();
		});
	}
	this->setObjectName("PostForm");
	setFontSize(settings.value("fontSize",14).toInt());
	ui->name->installEventFilter(this);
	ui->email->installEventFilter(this);
	ui->com->installEventFilter(this);
	ui->browse->installEventFilter(this);
	ui->response->installEventFilter(this);
	submitConnection = connect(ui->submit,&QPushButton::clicked,this,&PostForm::postIt);
	setShortcuts();
	connect(&captcha,&Captcha::questionInfo,this,&PostForm::loadCaptchaQuestion);
	connect(&captcha,&Captcha::challengeInfo,this,&PostForm::loadCaptchaImage);
	connect(ui->challenge,&ClickableLabel::clicked,[=]{
		if(captchaTimer && captchaTimer->isActive()) captchaTimer->stop();
		captcha.loading = false;
		captcha.loaded = false;
		captcha.getCaptcha();
		ui->response->setFocus();
	});
}

void PostForm::loadCaptchaQuestion(QString &challenge){
	ui->question->show();
	qDebug() << "setting captcha info/question:" << challenge;
	ui->question->setText(challenge);
	ui->refreshCaptcha->show();
	ui->response->show();
}

void PostForm::loadCaptchaImage(QString &challenege, QPixmap &challengeImage){
	(void)challenege;
	ui->challenge->setMinimumSize(280,280);
	ui->challenge->show();
	qDebug() << "setting captcha image";
	ui->challenge->setPixmap(challengeImage);
	if(captchaTimer && captchaTimer->isActive()) captchaTimer->stop();
	captchaTimer->start(120000);
}

void PostForm::load(Chan *api, QString &board, QString thread)
{
	this->api = api;
	if(api->usesCaptcha()) captcha.startUp(api);
	this->board = board;
	this->thread = thread;
	if(thread.isEmpty())
		ui->subject->installEventFilter(this);
	else
		ui->subject->hide();
	this->setWindowTitle("post to /" + board + "/" + thread);
	ui->com->setFocus();
}

void PostForm::usePass(bool use4chanPass){
	if(api->usesCaptcha() && use4chanPass){
		ui->captcha->hide();
	}
	else if(api->usesCaptcha()){
		ui->captcha->show();
		ui->challenge->hide();
		ui->refreshCaptcha->hide();
	}
}

PostForm::~PostForm()
{
	if(captchaTimer && captchaTimer->isActive()) captchaTimer->stop();
	disconnect(postConnection);
	disconnect(flagsConnection);
	disconnect(captchaConnection);
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


//TODO captcha object should do this
void PostForm::verifyCaptcha(){
	QUrlQuery postData;
	postData.addQueryItem("c", captcha.challenge);
	QString response = captcha.easyCaptcha(ui->response->text());
	for(int i=0;i<response.length();i++){
		postData.addQueryItem("response",response.at(i));
	}
	QNetworkRequest request(QUrl(api->captchaLinks().challengeURL));
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
	request.setRawHeader(QByteArray("Referer"),api->captchaLinks().challengeURL.toUtf8());
	captchaReply = nc->captchaManager->post(request, postData.toString(QUrl::FullyEncoded).toUtf8());
	//possible segfault if threadtab/boardtab closes while waiting
	captchaConnection = connect(captchaReply,&QNetworkReply::finished,[=]{
		captchaReply->deleteLater();
		if(captchaTimer && captchaTimer->isActive()) captchaTimer->stop();
		if(captchaReply->error()){
			ui->question->setText("Error: try again");
			qDebug() << "Error:" << captchaReply->errorString();
			captcha.loading = false;
			captcha.loaded = false;
			captcha.getCaptcha();
		}
		else{
			QByteArray reply = captchaReply->readAll();
			ui->response->clear();
			if(reply.contains("fbc-success")){
				QString matchText = "readonly>";
				int start = reply.indexOf(matchText);
				int end = reply.indexOf("</textarea>",start+matchText.length());
				captchaCode = reply.mid(start+matchText.length(),end-start-matchText.length());
				qDebug() << captchaCode;
				ui->question->setText("Verified");
				ui->challenge->hide();
				ui->refreshCaptcha->hide();
				ui->response->hide();
				ui->submit->setFocus();
				captchaTimer->start(120000);
			}
			else{
				ui->question->setText("Success but need more: try again");
				captcha.loading = false;
				captcha.loaded = false;
				captcha.getCaptcha();
			}

		}
	});
}

void PostForm::postIt()
{
	QSettings settings;
	if(api->usesCaptcha() && settings.value("use4chanPass", false).toBool() == false && captchaCode.isEmpty()){
		captcha.getCaptcha();
		ui->response->setFocus();
		return;
	}
	this->removeEventFilter(this);
	addOverlay();
	disconnect(submitConnection);
	qDebug().noquote() << "posting";
	QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

	QHttpPart mode;
	mode.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"mode\""));
	mode.setBody("regist");
	multiPart->append(mode);

	if(!thread.isEmpty()){
		QHttpPart resto;
		resto.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"resto\""));
		resto.setBody(thread.toStdString().c_str());
		multiPart->append(resto);
	}
	else if(!ui->subject->text().isEmpty()){
		QHttpPart sub;
		sub.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"sub\""));
		sub.setBody(thread.toStdString().c_str());
		multiPart->append(sub);
	}
	if(!ui->name->text().isEmpty()){
		QHttpPart name;
		name.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"name\""));
		name.setBody(ui->name->text().toStdString().c_str());
		multiPart->append(name);
	}
	if(!ui->email->text().isEmpty()){
		QHttpPart email;
		email.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"email\""));
		email.setBody(ui->email->text().toStdString().c_str());
		multiPart->append(email);
	}
	if(!ui->com->toPlainText().isEmpty()){
		QHttpPart com;
		com.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"com\""));
		qDebug().noquote() << ui->com->toPlainText().toStdString().c_str();
		com.setBody(ui->com->toPlainText().toStdString().c_str());
		multiPart->append(com);
	}

	if(api->usesCaptcha() && settings.value("use4chanPass", false).toBool() == false){
		QHttpPart captchaResponse;
		captchaResponse.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"g-recaptcha-response\""));
		if(captchaCode.isEmpty()){
			multiPart->deleteLater();
			return;
		}
		captchaResponse.setBody(captchaCode.toStdString().c_str());
		multiPart->append(captchaResponse);
	}

	if(!filename.isEmpty()) {
		QHttpPart uploadFile;
		uploadFile.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/octet-stream"));
		uploadFile.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"upfile\"; filename=\""+filename+"\""));
		QFile *upQFile = new QFile(filename);
		upQFile->open(QIODevice::ReadOnly);
		uploadFile.setBodyDevice(upQFile);
		upQFile->setParent(multiPart);
		multiPart->append(uploadFile);
	}

	QUrl url = QUrl(api->postURL(board));
	QNetworkRequest request(url);
	postReply = nc->postManager->post(request, multiPart);
	postConnection = connect(postReply, &QNetworkReply::finished, this, &PostForm::postFinished);
	multiPart->setParent(postReply); // delete the multiPart with the reply
	isPosting=true;
}

void PostForm::postFinished()
{
	isPosting = false;
	captcha.loaded = false;
	captchaCode = "";
	if(captchaTimer && captchaTimer->isActive()) captchaTimer->stop();
	if(postReply->error()){
		qDebug() << postReply->errorString();
		overlay->displayText = "4chan error?";
	}
	else{
		QByteArray replyArray(postReply->readAll());
		QTextEdit *replyTextEdit = new QTextEdit();
		replyTextEdit->setHtml(replyArray);
		QString replyString = replyTextEdit->toPlainText();
		qDebug() << replyString;
		if(replyString.contains(QRegularExpression("uploaded.$|Post successful!$")))
		{
			delete replyTextEdit;
			overlay->displayText = replyString;
			ui->com->clear();
			setFilenameText(empty);
			filename = "";
			ui->cancel->hide();
			QTimer::singleShot(1000, this, &PostForm::close);
			QRegularExpression re("<!-- thread:(?<replyThreadNum>\\d+),no:(?<threadNum>\\d+)");
			QRegularExpressionMatch match = re.match(replyArray);
			if(thread == ""){
				if(match.hasMatch()){
					QString captured(match.captured("threadNum"));
					you.addYou(api->name(),board,captured);
					qDebug() << "post successful; loading thread:" << match.captured("threadNum");
					emit loadThread(captured);
					postExtraFlags(captured);
				}
				else{
					qDebug() << "post succesful; but some other error";
					qDebug() << replyString;
				}
			}
			else{
				if(match.hasMatch()){
					QString captured(match.captured("threadNum"));
					you.addYou(api->name(),board,captured);
					postExtraFlags(captured);
				}
			}
		}
		else{
			qDebug() << "showing reply";
			replyTextEdit->setAttribute(Qt::WA_DeleteOnClose);
			connect(replyTextEdit,&QTextEdit::destroyed,[=]{
				qDebug() << "response window destroyed";
			});
			replyTextEdit->setWindowTitle("post to /"+board+"/"+thread+" response");
			replyTextEdit->setMinimumSize(800,600);
			replyTextEdit->setGeometry(0,0,this->width(),this->height());
			qDebug().noquote() << replyArray;
			replyTextEdit->show();
		}
	}
	postReply->deleteLater();
	QSettings settings;
	if(api->usesCaptcha() && settings.value("use4chanPass", false).toBool() == false){
		ui->response->show();
		ui->question->clear();
		ui->question->hide();
		ui->refreshCaptcha->hide();
	}
	QTimer::singleShot(1000, this,&PostForm::removeOverlay);
	this->installEventFilter(this);
	submitConnection = connect(ui->submit,&QPushButton::clicked,this,&PostForm::postIt);
}

void PostForm::postExtraFlags(const QString &postNum){
	QSettings settings;
	if(!settings.value("extraFlags/enable",false).toBool()
		|| !(board == "int" || board == "pol" || board == "sp" || board == "int")) return;
	QString region = settings.value("extraFlags/region",QString()).toString();
	if(region.isEmpty()) return;
	QUrlQuery postData;
	postData.addQueryItem("board",board);
	postData.addQueryItem("post_nr",postNum);
	postData.addQueryItem("regions",region);
	QNetworkRequest request(QUrl("https://flagtism.drunkensailor.org/int/post_flag_api2.php"));
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
	QNetworkReply *reply = nc->postManager->post(request, postData.toString(QUrl::FullyEncoded).toUtf8());
	flagsConnection = connect(reply,&QNetworkReply::finished,[=]{
		reply->deleteLater();
		if(!reply) return;
		if(reply->error()){
			qDebug().noquote() << "extraFlags: error posting flag regions:" << reply->errorString();
		}
		else{
			qDebug().noquote().nospace() << "extraFlags: postNum " << postNum << ", region: " << region << " set";
		}
	});
}

void PostForm::addOverlay()
{
	qDebug().noquote() << "adding overlay";
	overlay = new Overlay(this);
	overlay->show();
	overlay->installEventFilter(this);
	focused = this->focusWidget();
	/*ui->com->setFocusPolicy(Qt::NoFocus);
	ui->filename->setFocus();*/
}

void PostForm::removeOverlay()
{
	qDebug().noquote() << "removing overlay";
	delete overlay;
	ui->com->setFocusPolicy(Qt::StrongFocus);
	focused->setFocus();
}

//check isPosting before calling this
void PostForm::cancelPost(){
	qDebug() << "canceling post";
	postReply->abort();
	isPosting = false;
	captcha.loaded = false;
	captcha.loading = false;
	removeOverlay();
}

bool PostForm::eventFilter(QObject *obj, QEvent *event)
{
	if (event->type() == QEvent::KeyPress) {
		QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
		int mod = keyEvent->modifiers();
		int key = keyEvent->key();
		//qDebug("Ate key press %d", key);
		//qDebug("Ate modifier press %d", mod);
		//escape to cancel post or hide postform
		if(key == Qt::Key_Escape) {
			if(isPosting) {
				cancelPost();
			}
			else if(captcha.loading){
				captcha.cancel();
				ui->question->setText("Captcha load canceled");
				ui->response->show();
			}
			else {
				hide();
			}
			return true;
		}
		//shift+enter to post
		if(mod == 33554432 && key == Qt::Key_Return) {
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
			captcha.getCaptcha();
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
	if(filename.isEmpty()) {
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
	if(!filename.isEmpty()) setFilenameText(filename);
	else setFilenameText(empty);
	return QWidget::resizeEvent(event);
}

void PostForm::setFontSize(int fontSize){
	QFont temp = ui->com->font();
	temp.setPointSize(fontSize);
	ui->com->setFont(temp);
	temp.setPointSize(fontSize-2);
	ui->name->setFont(temp);
	ui->email->setFont(temp);
	ui->subject->setFont(temp);
	ui->response->setFont(temp);
	ui->browse->setFont(temp);
	ui->filename->setFont(temp);
	ui->submit->setFont(temp);
}

void PostForm::on_response_returnPressed()
{
	verifyCaptcha();
}

void PostForm::on_refreshCaptcha_clicked()
{
	if(captchaTimer && captchaTimer->isActive()) captchaTimer->stop();
	captcha.cancel();
	captcha.getCaptcha();
}
