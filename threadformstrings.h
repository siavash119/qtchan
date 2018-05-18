#ifndef THREADFORMSTRINGS_H
#define THREADFORMSTRINGS_H

#include "post.h"
#include "chans/chan.h"
#include <QString>
#include <QStringList>

class ThreadFormStrings
{
public:
	ThreadFormStrings();
	ThreadFormStrings(Chan *api, const Post &post, QString thread, QString path = "index");

	QString thread;
	QString path;
	QString board;
	QString pathBase;

	QString fileUrl;
	QString filePath;
	QString fileInfoString;

	QString thumbUrl;
	QString thumbPath;

	QString countryName;
	QString countryCode;
	QString flagUrl;
	QString flagPath;
	QString countryString;
	QString regionString;
	QStringList regionList;

	QString flegsUrlBase = "https://raw.githubusercontent.com/flaghunters/Extra-Flags-for-int-/master/flags/";

	QString name;
	QString realNow;
	QString no;

private:
	void flagStrings(const Post &post);
	QString makeCountryHtml(const QString &path, const QString &name);

};

#endif // THREADFORMSTRINGS_H
