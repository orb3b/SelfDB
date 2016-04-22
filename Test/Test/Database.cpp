#include "Database.h"

#include <QString>
#include <QJsonValue>
#include <QJsonArray>
#include <QFuture>
#include <QFutureSynchronizer>
#include <QtConcurrent/QtConcurrentRun>

#include <stdio.h>
#include <stdlib.h>

#include "Calendar.h"
#include "Console.h"
#include "Utilities.h"

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

            fact.amount = rand();
            fact.price = (double) rand() + (double) rand() / RAND_MAX;
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
