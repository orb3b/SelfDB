#ifndef CONSOLE_H
#define CONSOLE_H

#include <QTextStream>

class Console
{
public:
    Console();

    static QString readLine();
    static void writeLine(const QString &line = QString());

private:
    static Console &instance();

    QTextStream m_in;
    QTextStream m_out;
};

#endif // CONSOLE_H
