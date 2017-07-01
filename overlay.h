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
	QRect rect;
	const QColor color;
protected:
	void paintEvent(QPaintEvent*);
};

#endif // OVERLAY_H
