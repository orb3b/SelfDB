#ifndef UTILITIES_H
#define UTILITIES_H

#include <QString>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

class Utilities
{
public:    
    static bool loadJson(const QString &path, QJsonObject *options);
    static bool loadJson(const QString &path, QJsonArray *options);

private:
    static bool loadJson(const QString &path, QJsonDocument *document);
};

#endif // UTILITIES_H
