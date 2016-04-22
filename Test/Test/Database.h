#ifndef DATABASE_H
#define DATABASE_H

#include <QtGlobal>
#include <QJsonObject>
#include <QList>

#include "Schema.h"

struct Header {
    int calendarSize;
    int categoriesSize;
    int subCategoriesSize;
    int productNamesSize;
    int salesChecksSize;
    int factsSize;
};

struct CalendarGrouping {
    int year;
    int month;
    QList<qint64> timestamps;
    double sum;
    qint64 count;
    double avg() { return count != 0 ? sum / count : 0; }
};

typedef QString (*RandomStringFunc)();

class Database
{
public:
    Database(QJsonObject &config);
    ~Database();

    void query();
    void query1();
    void query2();
    void query3();

    void print();

    bool load();
    bool generate();

private:
    void cleanUp();

    void runQuery(int start, int end, QList<CalendarGrouping> *groupings);
    void insertGrouping(const CalendarGrouping &grouping, QList<CalendarGrouping> &groupings);

    QString randomString(int maxSize);
    static QString randomString(const QString &prefix, const QString &alph, int length);

    template<typename T>
    static void generateProductNames(T *data, int size, RandomStringFunc generator);

    template<typename T>
    void readData(T **data, int size, FILE *fp);

    void blockRead(void *dstBuf, size_t elementSize, size_t count, FILE *fp);
    void blockWrite(const void *dstBuf, size_t elementSize, size_t count, FILE *fp);

    QJsonObject &m_config;

    Header *m_header;
    CalendarRow *m_calendar;
    CategoryRow *m_categories;
    SubCategoryRow *m_subCategories;
    ProductNameRow *m_productNames;
    SalesCheckRow *m_salesChecks;
    FactRow *m_facts;
};

#endif // DATABASE_H
