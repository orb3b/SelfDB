#ifndef CALENDAR_H
#define CALENDAR_H

#include <QDate>
#include "Schema.h"

class Calendar
{
public:
    Calendar(const QString &from, const QString &to);

    int daysCount() const;
    CalendarRow next();

private:
    QDate m_from;
    QDate m_to;
    QDate m_current;
};

#endif // CALENDAR_H
