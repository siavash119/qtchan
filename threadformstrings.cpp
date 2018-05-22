#include "threadformstrings.h"
#include <QDir>

ThreadFormStrings::ThreadFormStrings(){}

ThreadFormStrings::ThreadFormStrings(Chan *api, const Post &post, QString thread, QString path) :
	thread(thread), path(path)
{
	board = post.board;
	pathBase = api->name() % '/' % board % '/' % path % '/';
	if(post.files.size() && !post.filedeleted) {
		PostFile file = post.files.at(0);
		fileUrl = api->imageURL(board,thread,file.tim,file.ext);
		filePath = pathBase%post.no%"-"%file.filename%file.ext;
		fileInfoString = file.filename % file.ext
				% " (" % QString("%1").arg(file.w)
				% "x" % QString("%1").arg(file.h)
				% ", " % QString("%1").arg(file.fsize/1024,0,'f',0)
				% " KB)";
		thumbUrl = api->thumbURL(board,thread,file.tim,file.ext);
		thumbPath = pathBase%"thumbs/"%post.no%"-"%file.filename%"s.jpg";
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
