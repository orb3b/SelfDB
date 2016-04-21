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
    row.day = m_current.day();
    row.month = m_current.month();
    row.year = m_current.year();
    row.timestamp = QDateTime(m_current).toMSecsSinceEpoch();

    m_current = m_current.addDays(1);

    return row;
}
