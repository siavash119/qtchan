#ifndef YOU_H
#define YOU_H

#include <QSet>
#include <QHash>
#include <QString>
#include <QRegularExpression>

class You
{
public:
	You();
	void addYou(const QString &api, const QString &board, const QString &post);
	bool hasYou(const QString &api, const QString &board, const QString &text);
	QRegularExpressionMatchIterator findYou(const QString &api, const QString &board, const QString &text);
	void updateRegExp(const QString &api, const QString &board);
	QHash< QString, QHash<QString,QRegularExpression> > matchYou;

	void loadYou(QString fileName = "you");
	void saveYou(QString fileName = "you");

private:
	QHash< QString, QHash< QString,QSet<QString> > > yourPosts;
};

extern You you;

#endif // YOU_H
