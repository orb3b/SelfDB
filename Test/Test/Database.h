#ifndef DATABASE_H
#define DATABASE_H

#include <QtGlobal>
#include <QJsonObject>

#include "Schema.h"

struct Header {
    int calendarSize;
};

class Database
{
public:
    Database();
    ~Database();

    void print();

    bool load(const QJsonObject &options);
    static bool generate(const QJsonObject &options);

private:
    void cleanUp();

    static void blockRead(void *dstBuf, size_t elementSize, size_t count, FILE *fp);

    Header *m_header;
    CalendarRow *m_calendar;
};

#endif // DATABASE_H
