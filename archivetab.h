#ifndef ARCHIVETAB_H
#define ARCHIVETAB_H

#include <QWidget>

namespace Ui {
class ArchiveTab;
}

class ArchiveTab : public QWidget
{
	Q_OBJECT

public:
	explicit ArchiveTab(QWidget *parent = 0);
	~ArchiveTab();
	QString board;
	void fillTable(QString board);
	void fillBoards();
	void clearBoards();
signals:
	void loadThread(QString threadString);
public slots:
	void threadClicked(int row, int column);
private slots:
	void boardClicked();
private:
	Ui::ArchiveTab *ui;
};

#endif // ARCHIVETAB_H
