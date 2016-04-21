#include "Utilities.h"

#include <QFile>

#include "Console.h"

bool Utilities::loadJson(const QString &path, QJsonObject *options)
{
    QJsonDocument document;
    if (!loadJson(path, &document))
        return false;

    *options = document.object();

    return true;
}

bool Utilities::loadJson(const QString &path, QJsonArray *options)
{
    QJsonDocument document;
    if (!loadJson(path, &document))
        return false;

    *options = document.array();

    return true;
}

bool Utilities::loadJson(const QString &path, QJsonDocument *document)
{
    QFile file(path);

    if (!file.open(QIODevice::ReadOnly)) {
        Console::writeLine(QString("Cannot open %1 json for reading").arg(path));
        return false;
    }

    QByteArray data = file.readAll();
    QJsonParseError error;
    *document = QJsonDocument::fromJson(data, &error);
    if (error.error != QJsonParseError::NoError)
    {
        Console::writeLine(QString("Cannot load %1 json. Parse error: %2").arg(path).arg(error.errorString()));
        return false;
    }

    return true;
}
