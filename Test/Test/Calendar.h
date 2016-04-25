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
    QString monthToString(int month) const;
    QString dayOfWeekToString(int dayOfWeek) const;

    QDate m_from;
    QDate m_to;
    QDate m_current;
};

#endif // CALENDAR_H
