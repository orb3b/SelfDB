#include "Server.h"

#include <QDateTime>
#include <QFile>

#include "Console.h"
#include "Database.h"

Server::Server()
{

}

void Server::run()
{
    Database db;
    while (1) {
        // Read a acommand
        Console::writeLine("Input a command...");
        QString input = Console::readLine();

        // Start benchmarking
        qint64 timeStart = QDateTime::currentMSecsSinceEpoch();

        // Execute command
        QStringList args = input.split(" ", QString::SkipEmptyParts);
        QString command = args.length() > 0 ? args[0] : "";
        if (command == "exit") {
            break;
        }
        if (command == "generate") {
            generateCommand(args);
        }
        if (command == "load") {
            db.load(QJsonObject());
        }
        if (command == "print") {
            db.print();
        }

        // Print benchmark results
        qint64 timeEnd = QDateTime::currentMSecsSinceEpoch();
        Console::writeLine();
        Console::writeLine(QString("Time elapsed: %1").arg(timeEnd - timeStart));
        Console::writeLine();
    }
}

void Server::generateCommand(const QStringList &args)
{
    QString fileName = args.length() > 1 ? args[1] : "generate.json";

    QJsonObject options;
    if (!loadJson(fileName, &options))
    {
        return;
    }

    Database::generate(options);
}

bool Server::loadJson(const QString &path, QJsonObject *options)
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
