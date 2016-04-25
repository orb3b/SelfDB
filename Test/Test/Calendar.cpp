#include "Calendar.h"

Calendar::Calendar(const QString &from, const QString &to)
{
    m_from = QDate::fromString(from, "dd.MM.yyyy");
    m_to = QDate::fromString(to, "dd.MM.yyyy");
    m_current = m_from;
}

int Calendar::daysCount() const
{
    return m_from.daysTo(m_to);
}

CalendarRow Calendar::next()
{
    CalendarRow row;
    row.timestamp = QDateTime(m_current).toMSecsSinceEpoch();
    row.year = m_current.year();
    row.halfyear = m_current.month() / 6 + 1;
    row.quater = m_current.month() / 4 + 1;
    row.monthNumber = m_current.month();
    strcpy(row.month, monthToString(m_current.month()).toLocal8Bit().data());
    row.weekNumber = m_current.dayOfWeek();
    strcpy(row.weekDay, dayOfWeekToString(m_current.dayOfWeek()).toLocal8Bit().data());
    row.dayNumber = m_current.dayOfYear();
    row.day = m_current.day();
    strcpy(row.daysTime, "Evening");

    m_current = m_current.addDays(1);

    return row;
}

QString Calendar::monthToString(int month) const
{
    switch (month) {
        case 1: return "January";
        case 2: return "February";
        case 3: return "March";
        case 4: return "April";
        case 5: return "May";
        case 6: return "June";
        case 7: return "July";
        case 8: return "August";
        case 9: return "September";
        case 10: return "October";
        case 11: return "November";
        case 12: return "December";
        default: return "January";
    }
}

QString Calendar::dayOfWeekToString(int dayOfWeek) const
{
    switch (dayOfWeek) {
        case 1: return "Monday";
        case 2: return "Tuesday";
        case 3: return "Wednesday";
        case 4: return "Thursday";
        case 5: return "Friday";
        case 6: return "Saturday";
        case 7: return "Sunday";
        default: return "Sunday";
    }
}
