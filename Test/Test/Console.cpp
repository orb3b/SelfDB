#include "Console.h"

Console::Console() :
    m_in(stdin),
    m_out(stdout)
{

}

QString Console::readLine()
{
    return Console::instance().m_in.readLine();
}

void Console::writeLine(const QString &line)
{
    Console::instance().m_out << line << endl;
}

Console &Console::instance()
{
    static Console console;
    return console;
}
