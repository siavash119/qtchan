#include "settings.h"
#include "ui_settings.h"
#include <QSettings>
#include <QFile>
#include <QDebug>

Settings::Settings(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::Settings)
{
	ui->setupUi(this);
	refreshValues();
	connect(ui->autoExpandLabel,&ClickableLabel::clicked,this,&Settings::clicked);
	connect(ui->autoUpdateLabel,&ClickableLabel::clicked,this,&Settings::clicked);
	connect(ui->sessionFileLabel,&ClickableLabel::clicked,this,&Settings::clicked);
	connect(ui->showIndexRepliesLabel,&ClickableLabel::clicked,this,&Settings::clicked);
	connect(ui->use4chanPassLabel,&ClickableLabel::clicked,this,&Settings::clicked);
	connect(ui->autoExpand,&QCheckBox::clicked,this,&Settings::checked);
	connect(ui->autoUpdate,&QCheckBox::clicked,this,&Settings::checked);
	connect(ui->showIndexReplies,&QCheckBox::clicked,this,&Settings::checked);
	connect(ui->use4chanPass,&QCheckBox::clicked,this,&Settings::checked);
}

Settings::~Settings()
{
	delete ui;
}

void Settings::checked(bool checked){
	(void)checked;
	QSettings settings;
	QObject *obj = sender();
	QString sender = obj->objectName();
	qDebug() << sender;
	if(sender == "autoExpand") {
		bool autoExpand = !settings.value("autoExpand", !ui->autoExpand->isChecked()).toBool();
		qDebug () << "setting autoExpand to" << autoExpand;
		settings.setValue("autoExpand",autoExpand);
		emit update("autoExpand", autoExpand);
		//refreshValues();
	}
	else if(sender == "autoUpdate") {
		bool autoUpdate = !settings.value("autoUpdate", !ui->autoUpdate->isChecked()).toBool();
		qDebug () << "setting autoUpdate to" << autoUpdate;
		settings.setValue("autoUpdate",autoUpdate);
		emit update("autoUpdate", autoUpdate);
		//refreshValues();
	}
	else if(sender == "showIndexReplies") {
		bool showIndexReplies = !settings.value("showIndexReplies", !ui->showIndexReplies->isChecked()).toBool();
		qDebug () << "setting showIndexReplies to" << showIndexReplies;
		settings.setValue("showIndexReplies",showIndexReplies);
		emit update("showIndexReplies", showIndexReplies);
		//refreshValues();
	}
	else if(sender == "use4chanPass"){
		bool use4chanPass = !settings.value("use4chanPass",!ui->use4chanPass->isChecked()).toBool();
		qDebug () << "setting use4chanPass to" << use4chanPass;
		settings.setValue("use4chanPass",use4chanPass);
		emit update("use4chanPass", use4chanPass);
	}
}

void Settings::clicked()
{
	QSettings settings;
	QObject *obj = sender();
	QString sender = obj->objectName();
	qDebug() << sender;
	if(sender == "sessionFileLabel") {
		QString sessionFile = ui->sessionFile->text();
		qDebug () << "setting sessionFile to" << sessionFile;
		settings.setValue("sessionFile",sessionFile);
		emit update("sessionFile", sessionFile);
		//refreshValues();
	}
	else if(sender == "autoExpandLabel") {
		bool autoExpand = !settings.value("autoExpand", !ui->autoExpand->isChecked()).toBool();
		qDebug () << "setting autoExpand to" << autoExpand;
		settings.setValue("autoExpand",autoExpand);
		emit update("autoExpand", autoExpand);
		ui->autoExpand->setChecked(autoExpand);
	}
	else if(sender == "autoUpdateLabel") {
		bool autoUpdate = !settings.value("autoUpdate", !ui->autoUpdate->isChecked()).toBool();
		qDebug () << "setting autoUpdate to" << autoUpdate;
		settings.setValue("autoUpdate",autoUpdate);
		emit update("autoUpdate", autoUpdate);
		ui->autoUpdate->setChecked(autoUpdate);
	}
	else if(sender == "showIndexRepliesLabel") {
		bool showIndexReplies = !settings.value("showIndexReplies", !ui->showIndexReplies->isChecked()).toBool();
		qDebug () << "setting showIndexReplies to" << showIndexReplies;
		settings.setValue("showIndexReplies",showIndexReplies);
		emit update("showIndexReplies", showIndexReplies);
		ui->showIndexReplies->setChecked(showIndexReplies);
	}
	else if(sender == "use4chanPassLabel") {
		bool use4chanPass = !settings.value("use4chanPass", !ui->use4chanPass->isChecked()).toBool();
		qDebug () << "setting use4chanPass to" << use4chanPass;
		settings.setValue("use4chanPass",use4chanPass);
		emit update("use4chanPass", use4chanPass);
		ui->use4chanPass->setChecked(use4chanPass);
	}
}

void Settings::refreshValues()
{
	QSettings settings;
	ui->autoExpand->setChecked(settings.value("autoExpand",false).toBool());
	ui->autoUpdate->setChecked(settings.value("autoUpdate",false).toBool());
	ui->showIndexReplies->setChecked(settings.value("showIndexReplies",false).toBool());
	ui->sessionFile->setText(settings.value("sessionFile","session.txt").toString());
	ui->use4chanPass->setChecked(settings.value("use4chanPass",false).toBool());
}

void Settings::showEvent(QShowEvent *event)
{
	(void)event;
	refreshValues();
}

