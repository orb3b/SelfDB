#ifndef UTILITIES_H
#define UTILITIES_H

#include <QString>
#include <QJsonDocument>
#include <QJsonObject>

class Utilities
{
public:    
    static bool loadJson(const QString &path, QJsonObject *options);
};

#endif // UTILITIES_H
