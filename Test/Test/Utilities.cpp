#include "Utilities.h"

#include <QFile>

#include "Console.h"

bool Utilities::loadJson(const QString &path, QJsonObject *options)
{
    QFile file(path);

    if (!file.open(QIODevice::ReadOnly)) {
        Console::writeLine(QString("Cannot open %1 json for reading").arg(path));
        return false;
    }

    QByteArray data = file.readAll();
    QJsonDocument document = QJsonDocument::fromJson(data);

    *options = document.object();

    return true;
}
