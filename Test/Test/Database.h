#ifndef DATABASE_H
#define DATABASE_H

#include <QtGlobal>
#include <QJsonObject>

#include "Schema.h"

struct Header {
    int calendarSize;
    int categoriesSize;
};

class Database
{
public:
    Database(QJsonObject &config);
    ~Database();

    void print();

    bool load();
    bool generate();

private:
    void cleanUp();

    void blockRead(void *dstBuf, size_t elementSize, size_t count, FILE *fp);
    void blockWrite(const void *dstBuf, size_t elementSize, size_t count, FILE *fp);

    QJsonObject &m_config;

    Header *m_header;
    CalendarRow *m_calendar;
};

#endif // DATABASE_H
