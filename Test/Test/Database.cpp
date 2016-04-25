#include "Database.h"

#include <QString>
#include <QJsonValue>
#include <QJsonArray>
#include <QFuture>
#include <QFutureSynchronizer>
#include <QtConcurrent/QtConcurrentRun>

#include <stdio.h>
#include <stdlib.h>
#include <list>

#include "Calendar.h"
#include "Console.h"
#include "Utilities.h"

using namespace std;

Database::Database(QJsonObject &config) :
    m_config(config),
    m_header(nullptr),
    m_calendar(nullptr),
    m_categories(nullptr),
    m_subCategories(nullptr),
    m_productNames(nullptr),
    m_salesChecks(nullptr),
    m_facts(nullptr)
{

}

Database::~Database()
{
    cleanUp();
}

void Database::query()
{
}

void Database::query1()
{
    auto found = false;
    for (auto i = 0; i < m_header->factsSize; i++) {
        auto fact = m_facts[i];

        CalendarRow date;
        if (!findInCalendar(fact.timestamp, &date))
            continue;

        if (date.year != 2020)
            continue;

        if (fact.id_subCategory == 48 && fact.id_category == 14)
        {
            found = true;
            break;
        }
    }
    Console::writeLine(QString("Found: %1").arg(found));
}

void Database::query2()
{
    auto threadCount = m_config["threadCount"].toInt();

    QFutureSynchronizer<void> synchronizer;

    QList<QList<CalendarGrouping>*> listOfGroupings;

    auto blockSize = m_header->factsSize / threadCount;
    auto start = 0;
    auto end = 0;
    for (auto i = 0; i < threadCount; i++) {
        start = end;
        auto amount = blockSize;
        if (i == threadCount - 1) {
            if (m_header->factsSize % 2 != 0)
            {
                amount = blockSize + 1;
                continue;
            }
        }
        end = start + amount;

        QList<CalendarGrouping> *lg = new QList<CalendarGrouping>();
        listOfGroupings.append(lg);
        synchronizer.addFuture(QtConcurrent::run(this, &Database::runQuery2,
                                                 start,
                                                 end,
                                                 lg
                                                 )
                               );
    }

    synchronizer.waitForFinished();

    // Show result
    QList<CalendarGrouping> result;
    for (auto i = 0; i < threadCount; i++) {
        QList<CalendarGrouping> &groupingList = *(listOfGroupings[i]);
        foreach (auto grouping, groupingList) {
            auto found = false;
            for (auto j = 0; j < result.length(); j++) {
                CalendarGrouping &resultGrouping = result[j];
                if (resultGrouping.month == grouping.month &&
                    resultGrouping.year == grouping.year) {

                    resultGrouping.count += grouping.count;
                    resultGrouping.sum += grouping.sum;

                    found = true;
                    break;
                }
            }
            if (!found) {
                insertGrouping(grouping, result);
            }
        }
    }
    foreach (auto lg, listOfGroupings) {
        delete lg;
    }

    foreach (auto grouping, result) {
        Console::writeLine(QString("%1 %2 %3").arg(grouping.year).arg(grouping.month).arg(grouping.avg()));
    }
}

void Database::runQuery2(int start, int end, QList<CalendarGrouping> *groupings)
{
    // Group by and sort years
    for (auto i = 0; i < m_header->calendarSize; i++) {
        auto row = m_calendar[i];

        auto found = false;
        for (auto j = 0; j < groupings->length(); j++) {
            CalendarGrouping &grouping = (*groupings)[j];
            if (grouping.month == row.month &&
                grouping.year == row.year) {
                grouping.timestamps.append(row.timestamp);
                found = true;
            }
        }
        if (!found) {
            CalendarGrouping grouping;
            grouping.month = row.month;
            grouping.year = row.year;
            grouping.timestamps.append(row.timestamp);
            grouping.count = 0;
            grouping.sum = 0;

            insertGrouping(grouping, *groupings);
        }
    }

    for (auto j = 0; j < groupings->length(); j++) {
        CalendarGrouping &grouping = (*groupings)[j];
        grouping.minTimestamp = -1;
        grouping.maxTimestamp = -1;
        foreach (auto timestamp, grouping.timestamps) {
            // Get min
            if (grouping.minTimestamp < 0) {
                grouping.minTimestamp = timestamp;
            }

            if (timestamp < grouping.minTimestamp)
                grouping.minTimestamp = timestamp;

            // Get max
            if (grouping.maxTimestamp < 0) {
                grouping.maxTimestamp = timestamp;
            }

            if (timestamp > grouping.maxTimestamp)
                grouping.maxTimestamp = timestamp;
        }
    }

    // Main cycle
    for (auto i = start; i < end; i++) {
        auto fact = m_facts[i];
        for (auto j = 0; j < groupings->length(); j++) {
            CalendarGrouping &grouping = (*groupings)[j];

            if (fact.timestamp < grouping.minTimestamp ||
                fact.timestamp > grouping.maxTimestamp)
                continue;

            auto found = false;
            foreach (auto timestamp, grouping.timestamps) {
                if (timestamp == fact.timestamp) {
                    found = true;
                    break;
                }
            }

            if (found) {
                // Do grouping
                grouping.sum += fact.totalPrice;
                grouping.count++;
                break;
            }
        }
    }
}

void Database::query3()
{
    auto threadCount = m_config["threadCount"].toInt();

    QFutureSynchronizer<void> synchronizer;

    QList<QList<Grouping>*> listOfGroupings;

    auto blockSize = m_header->factsSize / threadCount;
    auto start = 0;
    auto end = 0;
    for (auto i = 0; i < threadCount; i++) {
        start = end;
        auto amount = blockSize;
        if (i == threadCount - 1) {
            if (m_header->factsSize % 2 != 0)
            {
                amount = blockSize + 1;
                continue;
            }
        }
        end = start + amount;

        QList<Grouping> *lg = new QList<Grouping>();
        listOfGroupings.append(lg);
        synchronizer.addFuture(QtConcurrent::run(this, &Database::runQuery3,
                                                 start,
                                                 end,
                                                 lg
                                                 )
                               );
    }

    synchronizer.waitForFinished();

    // Show result
    QList<Grouping> result;
    for (auto i = 0; i < threadCount; i++) {
        QList<Grouping> &groupingList = *(listOfGroupings[i]);
        foreach (auto grouping, groupingList) {
            auto found = false;
            for (auto j = 0; j < result.length(); j++) {
                Grouping &resultGrouping = result[j];
                if (resultGrouping.category == grouping.category) {

                    mergeSubGroupings(resultGrouping, grouping);

                    found = true;
                    break;
                }
            }
            if (!found) {
                insertGrouping(grouping, result);
            }
        }
    }

    // Do top count
    QList<Grouping> topCountResult;
    auto topCount = 10;

    auto previousMax = -1;
    for (auto k = 0; k < topCount; k++) {
        auto currentMax = -1;
        auto maxGroupingIndex = -1;
        auto maxSubGroupingIndex = -1;

        Grouping grouping;

        for (auto i = 0; i < result.length(); i++) {
            auto grouping = result[i];
            for (auto j = 0; j < grouping.subGroupings.length(); j++) {
                auto subGrouping = grouping.subGroupings[j];
                auto avg = subGrouping.avg();

                if (previousMax > 0 && avg >= previousMax)
                    continue;

                if (avg > currentMax) {
                    currentMax = avg;
                    maxGroupingIndex = i;
                    maxSubGroupingIndex = j;
                }
            }
        }

        if (maxGroupingIndex >= 0 && maxSubGroupingIndex >= 0) {
            previousMax = currentMax;
            grouping.category = result[maxGroupingIndex].category;
            grouping.subGroupings.append(result[maxGroupingIndex].subGroupings[maxSubGroupingIndex]);
            topCountResult.append(grouping);
        }
    }

    foreach (auto grouping, topCountResult) {
        foreach (auto subGroupong, grouping.subGroupings) {
            Console::writeLine(QString("%1 %2 %3").arg(grouping.category).arg(subGroupong.category).arg(subGroupong.avg()));
        }
    }

    foreach (auto lg, listOfGroupings) {
        delete lg;
    }
}

void Database::runQuery3(int start, int end, QList<Grouping> *groupings)
{
    // Create sub-category groupings
    QList<Grouping> subGroupings;
    for (auto i = 0; i < m_header->subCategoriesSize; i++) {
        auto row = m_subCategories[i];

        auto found = false;
        for (auto j = 0; j < subGroupings.length(); j++) {
            Grouping &subGrouping = subGroupings[j];
            if (subGrouping.category == row.name) {
                subGrouping.keys.append(row.id);
                found = true;
            }
        }
        if (!found) {
            Grouping subGrouping;
            subGrouping.category = row.name;
            subGrouping.keys.append(row.id);
            subGrouping.count = 0;
            subGrouping.sum = 0;

            insertGrouping(subGrouping, subGroupings);
        }
    }
    for (auto j = 0; j < subGroupings.length(); j++) {
        Grouping &subGrouping = subGroupings[j];
        subGrouping.minKey = -1;
        subGrouping.maxKey = -1;
        foreach (auto key, subGrouping.keys) {
            // Get min
            if (subGrouping.minKey < 0) {
                subGrouping.minKey = key;
            }

            if (key < subGrouping.minKey)
                subGrouping.minKey = key;

            // Get max
            if (subGrouping.maxKey < 0) {
                subGrouping.maxKey = key;
            }

            if (key > subGrouping.maxKey)
                subGrouping.maxKey = key;
        }
    }

    // Group by categories
    for (auto i = 0; i < m_header->categoriesSize; i++) {
        auto row = m_categories[i];

        auto found = false;
        for (auto j = 0; j < groupings->length(); j++) {
            Grouping &grouping = (*groupings)[j];
            if (grouping.category == row.name) {
                grouping.keys.append(row.id);
                found = true;
            }
        }
        if (!found) {
            Grouping grouping;
            grouping.category = row.name;
            grouping.keys.append(row.id);
            grouping.count = 0;
            grouping.sum = 0;
            grouping.subGroupings = subGroupings;

            insertGrouping(grouping, *groupings);
        }
    }
    for (auto j = 0; j < groupings->length(); j++) {
        Grouping &grouping = (*groupings)[j];
        grouping.minKey = -1;
        grouping.maxKey = -1;
        foreach (auto key, grouping.keys) {
            // Get min
            if (grouping.minKey < 0) {
                grouping.minKey = key;
            }

            if (key < grouping.minKey)
                grouping.minKey = key;

            // Get max
            if (grouping.maxKey < 0) {
                grouping.maxKey = key;
            }

            if (key > grouping.maxKey)
                grouping.maxKey = key;
        }
    }

    // Main cycle
    for (auto i = start; i < end; i++) {
        auto fact = m_facts[i];

        CalendarRow date;
        if (!findInCalendar(fact.timestamp, &date))
            continue;

        if (date.year != 2020)
            continue;

        for (auto j = 0; j < groupings->length(); j++) {
            Grouping &grouping = (*groupings)[j];

            if (fact.id_category < grouping.minKey ||
                fact.id_category > grouping.maxKey)
                continue;

            auto found = false;
            foreach (auto key, grouping.keys) {
                if (key == fact.id_category) {
                    found = true;
                    break;
                }
            }

            if (found) {
                // Do grouping
                for (auto k = 0; k < grouping.subGroupings.length(); k++) {
                    Grouping &subGrouping = grouping.subGroupings[k];

                    if (fact.id_subCategory < subGrouping.minKey ||
                        fact.id_subCategory > subGrouping.maxKey)
                        continue;

                    auto foundSubGrouping = false;
                    foreach (auto subkey, subGrouping.keys) {
                        if (subkey == fact.id_subCategory) {
                            foundSubGrouping = true;
                            break;
                        }
                    }

                    if (foundSubGrouping) {
                        // Do grouping
                        subGrouping.sum += fact.totalPrice;
                        subGrouping.count++;
                        break;
                    }
                }
//                grouping.sum += fact.totalPrice;
//                grouping.count++;
                break;
            }
        }
    }
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

    Console::writeLine("Reading calendar...");
    readData(&m_calendar, m_header->calendarSize, fp);

    Console::writeLine("Reading categories...");
    readData(&m_categories, m_header->categoriesSize, fp);

    Console::writeLine("Reading sub-categories...");
    readData(&m_subCategories, m_header->subCategoriesSize, fp);

    Console::writeLine("Reading product names...");
    readData(&m_productNames, m_header->productNamesSize, fp);

    Console::writeLine("Reading sales checks...");
    readData(&m_salesChecks, m_header->salesChecksSize, fp);

    Console::writeLine("Reading facts...");
    readData(&m_facts, m_header->factsSize, fp);

    fclose(fp);

    return true;
}

bool Database::generate()
{
    QString fileName = m_config["databaseFile"].toString();
    auto bufferSize = m_config["bufferSize"].toInt();

    auto generationProperties = m_config["generation"].toObject();
    auto calendarProperties = generationProperties["calendar"].toObject();
    auto calendarFrom = calendarProperties["from"].toString();
    auto calendarTo = calendarProperties["to"].toString();

    auto categoriesProperties = generationProperties["categories"].toObject();
    auto categoriesSize = categoriesProperties["rowCount"].toInt();
    auto categoriesExamplesFile = categoriesProperties["examples"].toString();

    auto subCategoriesProperties = generationProperties["subCategories"].toObject();
    auto subCategoriesSize = subCategoriesProperties["rowCount"].toInt();
    auto subCategoriesExamplesFile = subCategoriesProperties["examples"].toString();

    auto productNamesProperties = generationProperties["productNames"].toObject();
    auto productNamesSize = productNamesProperties["rowCount"].toInt();

    auto salesChecksProperties = generationProperties["salesChecks"].toObject();
    auto salesChecksSize = salesChecksProperties["rowCount"].toInt();

    auto factsProperties = generationProperties["facts"].toObject();
    auto factsSize = factsProperties["rowCount"].toInt();

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
    header.subCategoriesSize = subCategoriesSize;
    header.productNamesSize = productNamesSize;
    header.salesChecksSize = salesChecksSize;
    header.factsSize = factsSize;
    // Write header
    fwrite(&header, sizeof(Header), 1, fp);

    // Generate calendar
    Console::writeLine("Generating calendar...");
    CalendarRow *calendarData = new CalendarRow[calendarSize];
    for (auto i = 0; i < calendarSize; i++) {
        calendarData[i] = calendar.next();
    }
    // Write calendar
    Console::writeLine("Writing calendar...");
    blockWrite(calendarData, sizeof(CalendarRow), calendarSize, fp);

    // Generate categories
    Console::writeLine("Generating categories...");
    QJsonArray categoriesExamples;
    Utilities::loadJson(categoriesExamplesFile, &categoriesExamples);

    CategoryRow *categoriesData = new CategoryRow[categoriesSize];
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
    Console::writeLine("Writing categories...");
    blockWrite(categoriesData, sizeof(CategoryRow), categoriesSize, fp);

    // Generate sub-categories
    Console::writeLine("Generating sub-categories...");
    QJsonArray subCategoriesExamples;
    Utilities::loadJson(subCategoriesExamplesFile, &subCategoriesExamples);

    SubCategoryRow *subCategoriesData = new SubCategoryRow[subCategoriesSize];
    for (auto i = 0; i < subCategoriesSize; i++) {
        subCategoriesData[i].id = i;

        QString value;
        int maxSize = sizeof(subCategoriesData[i].name) / sizeof(char);
        if (subCategoriesExamples.count() > 0) {
            int circles = i / subCategoriesExamples.count();
            int exampleIndex = i % subCategoriesExamples.count();

            if (circles > 0)
                value = QString("%1 %2").arg(subCategoriesExamples.at(exampleIndex).toString()).arg(circles).toLocal8Bit().data();
            else
                value = subCategoriesExamples.at(exampleIndex).toString();
        } else {
            value = randomString(maxSize - 5);
        }

        if (value.count() < maxSize)
            strcpy(subCategoriesData[i].name, value.toLocal8Bit().data());
    }
    // Write sub-categories
    Console::writeLine("Writing sub-categories...");
    blockWrite(subCategoriesData, sizeof(SubCategoryRow), subCategoriesSize, fp);

    // Generate product names and sales checks
    Console::writeLine("Generating product names and sales checks...");

    ProductNameRow *productNamesData = new ProductNameRow[productNamesSize];
    memset(productNamesData, 0, productNamesSize * sizeof(ProductNameRow));

    SalesCheckRow *salesChecksData = new SalesCheckRow[salesChecksSize];
    memset(salesChecksData, 0, salesChecksSize * sizeof(SalesCheckRow));

    QFutureSynchronizer<void> synchronizer;

    synchronizer.addFuture(QtConcurrent::run(Database::generateProductNames<ProductNameRow>,
                                             productNamesData,
                                             productNamesSize,
                                             []{ return Database::randomString("Name ", "1234567890", 30); })
                          );

    synchronizer.addFuture(QtConcurrent::run(Database::generateProductNames<SalesCheckRow>,
                                             salesChecksData,
                                             salesChecksSize,
                                             []{ return Database::randomString("#", "123567890", 30); })
                          );

    synchronizer.waitForFinished();

    // Write product names and sales checks
    Console::writeLine("Writing product names and sales checks...");
    blockWrite(productNamesData, sizeof(SubCategoryRow), productNamesSize, fp);
    blockWrite(salesChecksData, sizeof(SalesCheckRow), salesChecksSize, fp);

    // Generate facts
    Console::writeLine("Generating and writing facts...");

    size_t blockSize = bufferSize / sizeof(FactRow);
    FactRow *facts = new FactRow[blockSize];
    memset(facts, 0, sizeof(FactRow) * blockSize);

    size_t sizeWritten = 0;
    while (sizeWritten < factsSize) {
        size_t amount = 0;
        if (sizeWritten + blockSize < factsSize) {
            amount = blockSize;
        } else {
            amount = factsSize - sizeWritten;
        }

        for (auto i = 0; i < amount; i++) {
            FactRow fact;
            memset(&fact, 0, sizeof(FactRow));

            fact.amount = rand() % 100;
            fact.price = (double) (rand() % 10000) + (double) rand() / RAND_MAX;
            fact.discount = (double) rand() / RAND_MAX;
            fact.totalPrice = fact.price * fact.amount * (1 - fact.discount);

            fact.timestamp = calendarData[rand() % calendarSize].timestamp;
            fact.id_category = categoriesData[rand() % categoriesSize].id;
            fact.id_subCategory = subCategoriesData[rand() % subCategoriesSize].id;
            fact.id_productName = productNamesData[rand() % productNamesSize].id;
            fact.id_category = salesChecksData[rand() % salesChecksSize].id;

            facts[i] = fact;
        }
        fwrite(facts, sizeof(FactRow), amount, fp);

        sizeWritten += amount;
    }
    delete [] facts;

    // Cleanup
    delete [] calendarData;
    delete [] categoriesData;
    delete [] subCategoriesData;
    delete [] productNamesData;
    delete [] salesChecksData;

    fclose(fp);

    Console::writeLine("Done");

    return true;
}

void Database::cleanUp()
{
    delete m_header;
    delete [] m_calendar;
    delete [] m_categories;
    delete [] m_subCategories;
    delete [] m_productNames;
    delete [] m_salesChecks;
    delete [] m_facts;

    m_header = nullptr;
    m_calendar = nullptr;
    m_subCategories = nullptr;
    m_productNames = nullptr;
    m_salesChecks = nullptr;
    m_facts = nullptr;
}

void Database::insertGrouping(const CalendarGrouping &grouping, QList<CalendarGrouping> &groupings)
{
    auto inserted = false;
    for (auto j = 0; j < groupings.length(); j++) {
        if (grouping.year > groupings[j].year)
            continue;
        else if (grouping.year < groupings[j].year) {
            groupings.insert(j, grouping);
            inserted = true;
        } else {
            // Equal by year
            if (grouping.month > groupings[j].month) {
                continue;
            } else {
                groupings.insert(j, grouping);
                inserted = true;
            }
        }

        if (inserted)
            break;
    }

    if (!inserted)
        groupings.append(grouping);
}

void Database::insertGrouping(const Grouping &grouping, QList<Grouping> &groupings)
{
    auto inserted = false;
    for (auto j = 0; j < groupings.length(); j++) {
        if (grouping.category > groupings[j].category)
            continue;
        else {
            groupings.insert(j, grouping);
            inserted = true;
        }

        if (inserted)
            break;
    }

    if (!inserted)
        groupings.append(grouping);
}

void Database::mergeSubGroupings(Grouping &result, const Grouping &grouping)
{
    // Show result
    foreach (auto subGroup, grouping.subGroupings) {
        auto found = false;
        for (auto j = 0; j < result.subGroupings.length(); j++) {
            Grouping &resultSubGrouping = result.subGroupings[j];
            if (resultSubGrouping.category == subGroup.category) {

                resultSubGrouping.count += subGroup.count;
                resultSubGrouping.sum += subGroup.sum;

                found = true;
                break;
            }
        }
        if (!found) {
            insertGrouping(subGroup, result.subGroupings);
        }
    }
}

bool Database::findInCalendar(qint64 timestamp, CalendarRow *result)
{
    CalendarRow key;
    key.timestamp = timestamp;
    auto item = (CalendarRow *) bsearch(&key,
                                        m_calendar,
                                        m_header->calendarSize,
                                        sizeof(CalendarRow),
                                        Database::compareDates
                                        );
    if (item == NULL)
        return false;

    *result = *item;

    return true;
}

int Database::compareDates(const void *a, const void *b)
{
    CalendarRow *row1 = (CalendarRow *) a;
    CalendarRow *row2 = (CalendarRow *) b;
    if (row1->timestamp > row2->timestamp)
        return 1;
    if (row1->timestamp < row2->timestamp)
        return -1;
    return 0;
}

QString Database::randomString(int length)
{
    QString alph = "QWERTYUIOP";
    QString str;
    for (auto i = 0; i < length; i++) {
        str.append(alph[rand() % alph.length()]);
    }
    return str;
}

QString Database::randomString(const QString &prefix, const QString &alph, int length)
{
    QString result = prefix;
    auto offset = prefix.length();
    for (auto i = offset; i < length;  i++) {
        result.append(alph[rand() % alph.length()]);
    }
    return result;
}

template<typename T>
void Database::generateProductNames(T *data, int size, RandomStringFunc generator)
{
    for (auto i = 0; i < size; i++) {
        data[i].id = i;

        QString value;
        while (1) {
            value = generator();
            auto found = false;
            for (auto j = 0; j < size; j++) {
                if (value == data[j].name) {
                    found = true;
                    break;
                }
            }
            if (!found)
                break;
        }
        int maxSize = sizeof(data[i].name) / sizeof(char);
        if (value.count() < maxSize)
            strcpy(data[i].name, value.toLocal8Bit().data());
    }
}

template<typename T>
void Database::readData(T **data, int size, FILE *fp)
{
    *data = new T[size];
    Database::blockRead(*data, sizeof(T), size, fp);
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
