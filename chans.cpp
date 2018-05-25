#include "chans.h"
#include <QRegularExpressionMatch>

Chan* Chans::stringToType(QString &url){
	QString match = eightChanAPI->regURL().match(url).captured("url");
	if(!match.isEmpty()){
		url = match;
		return eightChanAPI;
	}
	match = twoChHkAPI->regURL().match(url).captured("url");
	if(!match.isEmpty()){
		url = match;
		return twoChHkAPI;
	}
	match = fourChanAPI->regURL().match(url).captured("url");
	if(!match.isEmpty()){
		url = match;
		return fourChanAPI;
	}
	return Q_NULLPTR;
}

Chan* Chans::get(QString &name){
	if(name == "4chan") return fourChanAPI;
	else if(name == "2ch.hk") return twoChHkAPI;
	else if(name == "8ch") return eightChanAPI;
	return Q_NULLPTR;
}

void Chans::deleteAPIs(){
	delete fourChanAPI;
	delete eightChanAPI;
	delete twoChHkAPI;
}
