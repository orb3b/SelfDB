#include "Server.h"

#include <QDateTime>
#include <QFile>

#include <time.h>
#include <stdlib.h>
#include <QCoreApplication>

#include "Console.h"
#include "Database.h"
#include "Utilities.h"

Server::Server(int argc, char *argv[]) :
    QCoreApplication(argc, argv),
    m_db(m_config)
{    
}

int Server::exec()
{
    QMetaObject::invokeMethod(this, "run", Qt::QueuedConnection);
    return QCoreApplication::exec();
}

void Server::run()
{
    /* initialize random seed: */
    srand(time(NULL));

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

    quit();
}

void Server::runCommand(const QString &command, const QStringList &args)
{
    // Update config
    if (!Utilities::loadJson("config.json", &m_config))
    {
        Console::writeLine("Cannot load config");
        return;
    }

    if (command == "query") {
        m_db.query();
    }
    if (command == "query1") {
        m_db.query1();
    }
    if (command == "query2") {
        m_db.query2();
    }
    if (command == "query3") {
        m_db.query3();
    }
    if (command == "print") {
        m_db.print();
    }
    if (command == "load") {
        m_db.load();
    }
    if (command == "generate") {
        m_db.generate();
    }
}
