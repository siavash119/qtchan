#ifndef POST_H
#define POST_H
#include <QString>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <time.h>

class Post
{
public:
    Post(QJsonObject &p,QString &board);
    ~Post();
    QString no;
    int resto;

    //Only on OP (resto == 0) when true
    bool sticky = false;
    bool closed = false;
    bool archived = false;
    int archived_on = 0; //Time when archived

    QString now; //Date and time (MM\/DD\/YY(Day)HH:MM (:SS on some boards), EST/EDT timezone)
    int time; //Unix timestamp

    //might not be present
    QString name;
    QString trip;
    QString id;
    QString capcode;
    QString country;
    QString country_name;
    QString sub; //Subject
    QString com; //Comment

    //Image only (tim != "")
    QString tim; //Renamed filename (Unix timestamp + milliseconds)
    QString filename; //Original filename
    QString ext; //File extension (.jpg|.png|.gif|.pdf|.swf|.webm)
    int fsize; //File size
    QString md5; //File MD5
    int w; //Image width
    int h; //Image Height
    int tn_w; //Thumbnail width
    int tn_h; //Thumbnail height
    bool filedeleted; //File deleted?
    bool spoiler; //Spoiler image?
    int custom_spoiler; //Custom spoilers? only if board has customs

    //only OPs (resto != 0)
    int omitted_posts; //# replies ommited
    int omitted_images; //# images ommited
    bool bumplimit; //Bump limit met?
    bool imagelimit; //Image limit met?
    QString capcode_replies; //only /q/ array of capcode type of post IDs ({"admin":[1234,1267]})
    int last_modified; //Time when last modified (UNIX timestamp)
    QString tag; //only /f/ Thread tag
    QString semantic_url; //Thread URL slug

    int since4pass; //Year 4chan Pass bought (YYYY)

    //custom strings

    QString board;
    /*QString fileURL;
    QString filePath;
    QString thumbURL;
    QString thumbPath;
    */
private:
    QString quoteColor(QString string);
};

#endif // POST_H
