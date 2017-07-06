#ifndef OVERLAY_H
#define OVERLAY_H
#include <QWidget>

class Overlay : public QWidget
{
	Q_OBJECT
public:
	explicit Overlay(QWidget *parent);
	QString displayText;
private:
	QWidget* parent;
	const QColor color;
	QRect rectToFill;
protected:
	void paintEvent(QPaintEvent *event);
};

#endif // OVERLAY_H
