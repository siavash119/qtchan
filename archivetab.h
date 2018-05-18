#ifndef ARCHIVETAB_H
#define ARCHIVETAB_H

#include <QWidget>

class QHBoxLayout;
namespace Ui {
class ArchiveTab;
}

class ArchiveTab : public QWidget
{
	Q_OBJECT

public:
	explicit ArchiveTab(QWidget *parent = 0);
	~ArchiveTab();
	QString api;
	QString board;
	void fillAPIs();
	void fillBoards();
	void fillTable(QString board);
	void clearLayout(QHBoxLayout *layout);
signals:
	void loadThread(QString threadString);
public slots:
	void tableClicked(int row, int column);
private slots:
	void apiClicked();
	void boardClicked();
private:
	Ui::ArchiveTab *ui;
};

#endif // ARCHIVETAB_H
