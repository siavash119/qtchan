#include "threadformstrings.h"
#include "post.h"
#include <QDir>

ThreadFormStrings::ThreadFormStrings(){}

ThreadFormStrings::ThreadFormStrings(const Post &post, QString thread, QString path) :
	thread(thread), path(path)
{
	board = post.board;
	pathBase = "./" % board % "/" % path % "/";
	if(!post.tim.isNull() && !post.filedeleted) {
		fileUrl = board % "/" % post.tim % post.ext;
		filePath = pathBase%post.no%"-"%post.filename%post.ext;
		fileInfoString = post.filename % post.ext
				% " (" % QString("%1").arg(post.w)
				% "x" % QString("%1").arg(post.h)
				% ", " % QString("%1").arg(post.fsize/1024,0,'f',0)
				% " KB)";
		thumbUrl = this->board % "/" % post.tim % "s.jpg";
		thumbPath = pathBase%"thumbs/"%post.no%"-"%post.filename%"s.jpg";
	}
	flagStrings(post);
}

void ThreadFormStrings::flagStrings(const Post &post){
	if(post.country_name.isEmpty()) return;
	if(!post.country.isEmpty()){
		countryName = post.country_name;
		countryCode = post.country.toLower();
		flagUrl = "https://s.4cdn.org/image/country/" % countryCode % ".gif";
		flagPath = "flags/" % countryCode % ".gif";
	}
	else{
		countryCode = post.troll_country.toLower();
		flagUrl = "https://s.4cdn.org/image/country/troll/" % post.troll_country.toLower() % ".gif";
		flagPath = "flags/troll/"+countryCode+".gif";
	}
	countryString = makeCountryHtml(flagPath,post.country_name);
}

QString ThreadFormStrings::makeCountryHtml(const QString &path, const QString &name){
	return "<img src=\"" % path % "\" width=\"32\" height=\"20\">"
	% " <span style=\"color:lightblue\">" % name % "</span> ";
}
