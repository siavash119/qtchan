#include "settings.h"
#include "ui_settings.h"
#include <QSettings>
#include <QFile>
#include <QDebug>

Settings::Settings(QTabWidget *parent) :
	QTabWidget(parent),
	ui(new Ui::Settings)
{
	ui->setupUi(this);
	bColorRegExp.setPattern("background-color[^:]*:\\s*(?<bColor>[^\\s;$]*)?");
	setTabColor();
	refreshValues();
	connect(ui->autoExpandLabel,&ClickableLabel::clicked,this,&Settings::clicked);
	connect(ui->sessionFileLabel,&ClickableLabel::clicked,this,&Settings::clicked);
	connect(ui->autoUpdateLabel,&ClickableLabel::clicked,this,&Settings::clicked);
	connect(ui->autoScrollActiveLabel,&ClickableLabel::clicked,this,&Settings::clicked);
	connect(ui->autoScrollBackgroundLabel,&ClickableLabel::clicked,this,&Settings::clicked);
	connect(ui->showIndexRepliesLabel,&ClickableLabel::clicked,this,&Settings::clicked);
	connect(ui->use4chanPassLabel,&ClickableLabel::clicked,this,&Settings::clicked);
	connect(ui->autoExpand,&QCheckBox::clicked,this,&Settings::clicked);
	connect(ui->autoUpdate,&QCheckBox::clicked,this,&Settings::clicked);
	connect(ui->autoScrollActive,&QCheckBox::clicked,this,&Settings::clicked);
	connect(ui->autoScrollBackground,&QCheckBox::clicked,this,&Settings::clicked);
	connect(ui->showIndexReplies,&QCheckBox::clicked,this,&Settings::clicked);
	connect(ui->use4chanPass,&QCheckBox::clicked,this,&Settings::clicked);
}

Settings::~Settings()
{
	delete ui;
}

void Settings::setTabColor(){
	QColor bColor;
	QSettings settings;
	bColor.setNamedColor(settings.value("style/MainWindow/background-color","#191919").toString());
	//darkness = darkness*qPow(0.8,replyLevel-1);
	bColor.setRgb(static_cast<int>(bColor.red()*0.8),
					static_cast<int>(bColor.green()*0.8),
					static_cast<int>(bColor.blue()*0.8));
	QString tabStyle = "QTabBar::tab{background-color:" % bColor.name() % ";padding:5px 10px; border:1px solid white}";
	bColor.setRgb(static_cast<int>(bColor.red()*0.8),
					static_cast<int>(bColor.green()*0.8),
					static_cast<int>(bColor.blue()*0.8));
	tabStyle = tabStyle % "QTabBar::tab:selected{background-color:" % bColor.name() % "}";
	this->setStyleSheet(tabStyle);
}

void Settings::setSetting(QString setting, QVariant value, QCheckBox *box){
	QSettings settings;
	qDebug().noquote() << "setting" << setting << "to" << value.toString();
	settings.setValue(setting,value);
	emit update(setting, value);
	if(box) box->setChecked(value.toBool());
}

void Settings::clicked()
{
	QSettings settings;
	QObject *obj = sender();
	QString sender = obj->objectName();
	if(sender == "sessionFileLabel")
		setSetting("sessionFile",ui->sessionFile->text());
	else if(sender == "autoExpandLabel" || sender == "autoExpand")
		setSetting("autoExpand",!settings.value("autoExpand").toBool(),ui->autoExpand);
	else if(sender == "autoUpdateLabel" || sender == "autoUpdate")
		setSetting("autoUpdate",!settings.value("autoUpdate").toBool(),ui->autoUpdate);
	else if(sender == "autoScrollActiveLabel" || sender == "autoScrollActive")
		setSetting("autoScrollActive",!settings.value("autoScrollActive").toBool(),ui->autoScrollActive);
	else if(sender == "autoScrollBackgroundLabel" || sender == "autoScrollBackground")
		setSetting("autoScrollBackground",!settings.value("autoScrollBackground").toBool(),ui->autoScrollBackground);
	else if(sender == "showIndexRepliesLabel" || sender == "showIndexReplies")
		setSetting("showIndexReplies",!settings.value("showIndexReplies").toBool(),ui->showIndexReplies);
	else if(sender == "use4chanPassLabel" || sender == "use4chanPass")
		setSetting("use4chanPass",!settings.value("use4chanPass").toBool(),ui->use4chanPass);
}

void Settings::refreshValues()
{
	QSettings settings;
	ui->autoExpand->setChecked(settings.value("autoExpand",false).toBool());
	ui->autoUpdate->setChecked(settings.value("autoUpdate",false).toBool());
	ui->autoScrollActive->setChecked(settings.value("autoScrollActive",false).toBool());
	ui->autoScrollBackground->setChecked(settings.value("autoScrollBackground",false).toBool());
	ui->showIndexReplies->setChecked(settings.value("showIndexReplies",false).toBool());
	ui->sessionFile->setText(settings.value("sessionFile","session").toString());
	ui->use4chanPass->setChecked(settings.value("use4chanPass",false).toBool());
	ui->styleMainWindowEdit->setText(settings.value("style/MainWindow","background-color: #191919; color:white").toString());
	ui->styleThreadFormEdit->setText(settings.value("style/ThreadForm","color:#bbbbbb;").toString());
}

void Settings::showEvent(QShowEvent *event)
{
	(void)event;
	refreshValues();
}




void Settings::on_sessionFile_editingFinished()
{
	QSettings settings;
	QString text = ui->sessionFile->text();
	if(!text.isEmpty()){
		qDebug().noquote() << "setting sessionFile to" << text;
		settings.setValue("sessionFile",text);
		emit update("sessionFile",text);
	}
}

void Settings::on_styleMainWindowEdit_editingFinished()
{
	QSettings settings;
	QString text = ui->styleMainWindowEdit->text();
	if(!text.isEmpty()){
		qDebug().noquote() << "setting style/MainWindow to" << text;
		settings.setValue("style/MainWindow",text);
		QRegularExpressionMatch match = bColorRegExp.match(text);
		if(match.hasMatch()){
			settings.setValue("style/MainWindow/background-color",match.captured("bColor"));
		}
		else settings.setValue("style/MainWindow/background-color",QString());
		setTabColor();
		emit update("style/MainWindow",text);
	}
}

void Settings::on_styleThreadFormEdit_editingFinished()
{
	QSettings settings;
	QString text = ui->styleThreadFormEdit->text();
	if(!text.isEmpty()){
		qDebug().noquote() << "setting style/ThreadForm to" << text;
		settings.setValue("style/ThreadForm",text);
		QRegularExpressionMatch match = bColorRegExp.match(text);
		if(match.hasMatch()){
			settings.setValue("style/ThreadForm/background-color",match.captured("bColor"));
		}
		else settings.setValue("style/ThreadForm/background-color",QString());
		emit update("style/ThreadForm",text);
	}

}
