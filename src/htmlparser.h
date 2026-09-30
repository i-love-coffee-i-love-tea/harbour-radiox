#ifndef HTMLPARSER_H
#define HTMLPARSER_H

#include <QString>
#include <QList>
#include <QDate>
#include <QTime>
#include <QVariantMap>

struct ScheduleSlot {
    int hour = 0;
    QString showName;
    QString slug;
    QString subtitle;
    bool isRepeat = false;
    bool isDito = false;
    QString sendetippUrl;
};

struct ProgramDay {
    QString dayLabel;
    QList<ScheduleSlot> entries;
};

struct Recording {
    int id = 0;
    QDate date;
    QTime time;
    QString showName;
    QString subtitle;
    QString playbackUrl;
};

struct Sendetipp {
    QString title;
    QString showName;
    QString showSlug;
    QString dateTime;
    QString description;
    QString imageUrl;
};

class HtmlParser
{
public:
    static QList<ProgramDay> parseProgramWeek(const QString &html, QString &weekLabel);
    static QList<Recording> parseRecordings(const QString &html);
    static QList<Sendetipp> parseSendetipps(const QString &html);
    static QVariantMap parseShowDetail(const QString &html);
};

#endif // HTMLPARSER_H