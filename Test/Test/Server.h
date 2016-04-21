#ifndef SERVER_H
#define SERVER_H

#include <QString>

#include "Database.h"

class Server
{
public:
    Server();

    void run();

private:
    void runCommand(const QString &command, const QStringList &args);

    QJsonObject m_config;
    Database m_db;
};

#endif // SERVER_H
