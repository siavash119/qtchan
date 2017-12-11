#ifndef SETTINGS_H
#define SETTINGS_H

#include <QWidget>
#include <QVariant>

namespace Ui {
class Settings;
}

class Settings : public QWidget
{
	Q_OBJECT
public:
	explicit Settings(QWidget *parent = 0);
	void refreshValues();
	~Settings();
signals:
	void update(QString setting, QVariant value);
public slots:
	void clicked();
	void checked(bool checked = false);
private:
	Ui::Settings *ui;
protected:
	void showEvent(QShowEvent *event);
};

#endif // SETTINGS_H
