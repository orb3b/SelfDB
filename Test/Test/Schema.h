#ifndef SCHEMA_H
#define SCHEMA_H

#include <QtGlobal>

struct CalendarRow {
    uint day;
    uint month;
    uint year;
    qint64 timestamp;
};

struct CategoryRow {
    uint id;
    char name[40];
};

struct SubCategoryRow {
    uint id;
    char name[100];
};

struct ProductNameRow {
    uint id;
    char name[100];
};

struct SalesCheckRow {
    uint id;
    char name[40];
};

struct FactRow {
    int amount;
    double price;
    double discount;
    double totalPrice;
    qint64 timestamp;
    int id_category;
    int id_subCategory;
    int id_productName;
    int id_salesCheck;
};

#endif // SCHEMA_H
