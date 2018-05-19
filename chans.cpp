#include "chans.h"
#include <QRegularExpressionMatch>

Chan* Chans::stringToType(QString &url){
	QString match = fourChanAPI->regURL().match(url).captured("url");
	if(!match.isEmpty()){
		url = match;
		return fourChanAPI;
	}
	match = eightChanAPI->regURL().match(url).captured("url");
	if(!match.isEmpty()){
		url = match;
		return eightChanAPI;
	}
	match = twoChHkAPI->regURL().match(url).captured("url");
	if(!match.isEmpty()){
		url = match;
		return twoChHkAPI;
	}
	return Q_NULLPTR;
}

void Chans::deleteAPIs(){
	delete fourChanAPI;
	delete eightChanAPI;
	delete twoChHkAPI;
}
