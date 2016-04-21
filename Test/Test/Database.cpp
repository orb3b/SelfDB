#include "Database.h"

#include <QString>
#include <QJsonValue>
#include <QJsonArray>

#include <stdio.h>
#include <stdlib.h>

#include "Calendar.h"
#include "Console.h"
#include "Utilities.h"

Database::Database(QJsonObject &config) :
    m_config(config),
    m_header(nullptr),
    m_calendar(nullptr)
{

}

Database::~Database()
{
    cleanUp();
}

void Database::print()
{
    if (!m_header || !m_calendar) {
        Console::writeLine("Cannot print calendar - calendar is null");
        return;
    }

    for (auto i = 0; i < m_header->calendarSize; i++) {
        auto row = m_calendar[i];
        Console::writeLine(QString("%1 %2 %3 %4").arg(row.day).arg(row.month).arg(row.year).arg(row.timestamp));
    }
}

bool Database::load()
{
    cleanUp();

    QString fileName = m_config["databaseFile"].toString();

    auto fp = fopen(fileName.toLocal8Bit().data(), "rb");
    if (!fp) {
        Console::writeLine(QString("Cannot open %1 for reading!").arg(fileName));
        return false;
    }

    fseek(fp, 0, SEEK_SET);

    m_header = new Header;
    fread(m_header, sizeof(Header), 1, fp);
    if (feof(fp)) {
        Console::writeLine("Cannot read calendar: Unexpected end of file");
        return false;
    }

    m_calendar = new CalendarRow[m_header->calendarSize];
    Database::blockRead(m_calendar, sizeof(CalendarRow), m_header->calendarSize, fp);

    fclose(fp);

    return true;
}

bool Database::generate()
{
    QString fileName = m_config["databaseFile"].toString();

    auto generationProperties = m_config["generation"].toObject();
    auto calendarProperties = generationProperties["calendar"].toObject();
    auto calendarFrom = calendarProperties["from"].toString();
    auto calendarTo = calendarProperties["to"].toString();

    auto categoriesProperties = generationProperties["categories"].toObject();
    auto categoriesSize = categoriesProperties["rowCount"].toInt();
    auto categoriesExamplesFile = categoriesProperties["examples"].toString();

    auto fp = fopen(fileName.toLocal8Bit().data(), "wb");
    if (!fp) {
        Console::writeLine(QString("Cannot open %1 for writing!").arg(fileName));
        return false;
    }

    Header header;
    Calendar calendar(calendarFrom, calendarTo);

    // Setup header
    int calendarSize = calendar.daysCount();
    header.calendarSize = calendarSize;
    header.categoriesSize = categoriesSize;
    // Write header
    fwrite(&header, sizeof(Header), 1, fp);

    // Generate calendar
    CalendarRow *calendarData = new CalendarRow[calendarSize];
    for (auto i = 0; i < calendarSize; i++) {
        calendarData[i] = calendar.next();
    }
    // Write calendar
    blockWrite(calendarData, sizeof(CalendarRow), calendarSize, fp);
    delete [] calendarData;

    // Generate categories
    QJsonArray categoriesExamples;
    Utilities::loadJson(categoriesExamplesFile, &categoriesExamples);

    CategoryRow *categoriesData = new CategoryRow[calendarSize];
    auto index = 0;
    for (auto i = 0; i < categoriesSize; i++) {
        categoriesData[i].id = i;

        QString value;
        int maxSize = sizeof(categoriesData[i].name) / sizeof(char);
        if (categoriesExamples.count() > 0) {
            int circles = i / categoriesExamples.count();
            int exampleIndex = i % categoriesExamples.count();

            if (circles > 0)
                value = QString("%1 %2").arg(categoriesExamples.at(exampleIndex).toString()).arg(circles).toLocal8Bit().data();
            else
                value = categoriesExamples.at(exampleIndex).toString();
        } else {
            value = randomString(maxSize - 5);
        }

        if (value.count() < maxSize)
            strcpy(categoriesData[i].name, value.toLocal8Bit().data());
    }
    // Write categories
    blockWrite(categoriesData, sizeof(CategoryRow), categoriesSize, fp);
    delete [] categoriesData;

    fclose(fp);

    return true;
}

void Database::cleanUp()
{
    delete m_header;
    delete [] m_calendar;

    m_header = nullptr;
    m_calendar = nullptr;
}

QString Database::randomString(int maxSize)
{
    QString alph = "QWERTYUIOP";
    QString str;
    for (auto i = 0; i < maxSize; i++) {
        str.append(alph[rand() % alph.length()]);
    }
    return str;
}

void Database::blockRead(void *dstBuf, size_t elementSize, size_t count, FILE *fp)
{
    auto blockSize = m_config["bufferSize"].toInt();

    size_t size = elementSize * count;
    size_t offset = 0;
    while(offset < size) {
        size_t amount = 0;
        if (offset + blockSize < size) {
            amount = blockSize;
        } else {
            amount = size - offset;
        }
        fread((char *)dstBuf + offset, amount, 1, fp);

        offset += amount;
    }
}

void Database::blockWrite(const void *dstBuf, size_t elementSize, size_t count, FILE *fp)
{
    auto blockSize = m_config["bufferSize"].toInt();

    size_t size = elementSize * count;
    size_t offset = 0;
    while(offset < size) {
        size_t amount = 0;
        if (offset + blockSize < size) {
            amount = blockSize;
        } else {
            amount = size - offset;
        }
        fwrite((char *)dstBuf + offset, amount, 1, fp);

        offset += amount;
    }
}
