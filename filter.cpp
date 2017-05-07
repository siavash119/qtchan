#include "filter.h"

Filter::Filter()
{
    //quotelink.setPattern("href=\\\"#p(\\d+)\\\"");
}

/*void Filter::htmlParse(QString search){

}*/

QSet<QString> Filter::findQuotes(QString post){
    QRegularExpression quotelink;
    quotelink.setPattern("href=\\\"#p(\\d+)\\\"");
    QRegularExpressionMatch quotelinkMatch;
    QRegularExpressionMatchIterator quotelinkMatches;
    quotelinkMatches = quotelink.globalMatch(post);
    QSet<QString> quotes;
    while (quotelinkMatches.hasNext()) {
        quotelinkMatch = quotelinkMatches.next();
        quotes.insert(QString(quotelinkMatch.captured(1)));
    }
    return quotes;
}

/*
QSet<QString> Filter::crossthread(QString search){
    QRegularExpression quotelink;
    quotelink.setPattern("href=\\\"#p(\\d+)\\\"");
    QRegularExpressionMatch quotelinkMatch;
    QRegularExpressionMatchIterator quotelinkMatches;
    quotelinkMatches = quotelink.globalMatch(post);
    QSet<QString> quotes;
    while (quotelinkMatches.hasNext()) {
        quotelinkMatch = quotelinkMatches.next();
        quotes.insert(QString(quotelinkMatch.captured(1)));
    }
    return quotes;
}*/
