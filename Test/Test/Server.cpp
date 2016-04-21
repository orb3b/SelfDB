#include "Server.h"

#include <QDateTime>
#include <QFile>

#include "Console.h"
#include "Database.h"

Server::Server() :
    m_db(m_config)
{

}

void Server::run()
{
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

        runCommand(command, args);

        // Print benchmark results
        qint64 timeEnd = QDateTime::currentMSecsSinceEpoch();
        Console::writeLine();
        Console::writeLine(QString("Time elapsed: %1ms").arg(timeEnd - timeStart));
        Console::writeLine();
    }
}

void Server::runCommand(const QString &command, const QStringList &args)
{
    // Update config
    if (!loadJson("config.json", &m_config))
    {
        Console::writeLine("Cannot load config");
        return;
    }

    if (command == "generate") {
        m_db.generate();
    }
    if (command == "load") {
        m_db.load();
    }
    if (command == "print") {
        m_db.print();
    }
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
