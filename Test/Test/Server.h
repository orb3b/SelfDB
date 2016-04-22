#ifndef SERVER_H
#define SERVER_H

#include <QCoreApplication>
#include <QString>

#include "Database.h"

class Server : QCoreApplication
{
    Q_OBJECT
public:
    Server(int argc, char *argv[]);

    int exec();

private slots:
    void run();

private:
    void runCommand(const QString &command, const QStringList &args);

    QJsonObject m_config;
    Database m_db;
};

#endif // SERVER_H
