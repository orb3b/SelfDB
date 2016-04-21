#ifndef SERVER_H
#define SERVER_H

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QString>

class Server
{
public:
    Server();

    void run();

private:
    void generateCommand(const QStringList &args);

    bool loadJson(const QString &path, QJsonObject *options);
};

#endif // SERVER_H
