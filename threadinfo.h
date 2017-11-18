#ifndef THREADINFO_H
#define THREADINFO_H

#include <QFrame>
#include <QMouseEvent>

namespace Ui {
class ThreadInfo;
}

class ThreadInfo : public QFrame
{
    Q_OBJECT

public:
    explicit ThreadInfo(QWidget *parent = 0);
    ~ThreadInfo();
    int posts = 0;
    int files = 0;
    int hidden = 0;
    int unseen = 0;
    void updateFields();

private:
    Ui::ThreadInfo *ui;
    QPoint offset;

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

};

#endif // THREADINFO_H
