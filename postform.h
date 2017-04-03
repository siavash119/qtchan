#ifndef POSTFORM_H
#define POSTFORM_H

#include <QWidget>
#include <QNetworkReply>
#include <QFileDialog>
#include <QMimeData>

namespace Ui {
class PostForm;
}

class PostForm : public QWidget
{
    Q_OBJECT

public:
    explicit PostForm(QString board, QString thead = "0", QWidget *parent = 0);
    ~PostForm();
    void postIt();
    QString board;
    QString thread;
    QString filename;
    QNetworkReply *postReply;
    QFileDialog *dialog;
    QMetaObject::Connection submitConnection;
    void fileChecker(const QMimeData *mimedata);
    //TODO links in com?

public slots:
    void appendText(QString text);

private:
    Ui::PostForm *ui;

private slots:
    void postFinished();
    void fileSelected(const QString &file);
    void on_browse_clicked();
    void on_cancel_clicked();
    void droppedItem();
    void setShortcuts();

protected:
    bool eventFilter(QObject *obj, QEvent *event);
};

#endif // POSTFORM_H
