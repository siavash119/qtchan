#ifndef CHANS_H
#define CHANS_H

#include "chans/fourchan.h"

class Chans
{
public:
	static Chan* stringToType(QString url);
	static void deleteAPIs();
};

extern Chan *fourChanAPI;
#endif // CHANS_H
