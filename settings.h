#ifndef SETTINGS_H
#define SETTINGS_H

#include <QTabWidget>
#include <QVariant>

namespace Ui {
class Settings;
}

class Settings : public QTabWidget
{
	Q_OBJECT
public:
	explicit Settings(QTabWidget *parent = 0);
	void refreshValues();
	~Settings();
signals:
	void update(QString setting, QVariant value);
public slots:
	void clicked();
private:
	Ui::Settings *ui;
protected:
	void showEvent(QShowEvent *event);
private slots:
	void on_sessionFile_editingFinished();
	void on_styleMainWindowEdit_editingFinished();
	void on_styleThreadFormEdit_editingFinished();
};

#endif // SETTINGS_H
