#ifndef DATABASE_H
#define DATABASE_H

#include <QtGlobal>
#include <QJsonObject>

#include "Schema.h"

struct Header {
    int calendarSize;
    int categoriesSize;
    int subCategoriesSize;
    int productNamesSize;
    int saleChecksSize;
    int factsSize;
};

typedef QString (*RandomStringFunc)();

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

    QString randomString(int maxSize);
    static QString randomString(const QString &prefix, const QString &alph, int length);

    template<typename T>
    static void generateProductNames(T *data, int size, RandomStringFunc generator);

    void blockRead(void *dstBuf, size_t elementSize, size_t count, FILE *fp);
    void blockWrite(const void *dstBuf, size_t elementSize, size_t count, FILE *fp);

    QJsonObject &m_config;

    Header *m_header;
    CalendarRow *m_calendar;
};

#endif // DATABASE_H
