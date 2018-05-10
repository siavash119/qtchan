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
	void addYou(const QString &boardName, const QString &threadNum);
	bool hasYou(const QString &boardName, const QString &text);
	QRegularExpressionMatchIterator findYou(const QString &boardName, const QString &text);
	void updateRegExp(const QString &boardName);
	QHash<QString,QRegularExpression> matchYou;

	QString youFileName = "you";
	void loadYou(QString fileName = "you");
	void saveYou(QString fileName = "you");

private:
	QHash< QString,QSet<QString> > yourPosts;
};

extern You you;

#endif // YOU_H
