#include "Database.h"

#include <QString>
#include <QJsonValue>

#include <stdio.h>
#include <stdlib.h>

#include "Calendar.h"
#include "Console.h"

Database::Database() :
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

bool Database::load(const QJsonObject &options)
{
    cleanUp();

    auto fileName = "data.bin";

    auto fp = fopen(fileName, "rb");
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

bool Database::generate(const QJsonObject &options)
{
    QString fileName = options["fileName"].toString();
    auto calendarFrom = options["calendar"].toObject()["from"].toString();
    auto calendarTo = options["calendar"].toObject()["to"].toString();

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
    // Write header
    fwrite(&header, sizeof(Header), 1, fp);

    // Generate calendar
    CalendarRow *calendarData = new CalendarRow[calendarSize];
    for (auto i = 0; i < calendarSize; i++) {
        calendarData[i] = calendar.next();
    }
    // Write calendar
    fwrite(calendarData, sizeof(CalendarRow), calendarSize, fp);
    delete [] calendarData;

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

void Database::blockRead(void *dstBuf, size_t elementSize, size_t count, FILE *fp)
{
    auto blockSize = 100;

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
