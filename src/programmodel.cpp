#include "programmodel.h"

ProgramModel::ProgramModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int ProgramModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;
    if (m_days.isEmpty()) return 0;
    return m_days.first().entries.count();
}

QVariant ProgramModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || m_days.isEmpty())
        return QVariant();

    int row = index.row();

    if (role >= Day0Role && role <= Day6Role) {
        int dayIdx = role - Day0Role;
        if (dayIdx < m_days.size() && row < m_days[dayIdx].entries.count()) {
            const ScheduleSlot &slot = m_days[dayIdx].entries[row];
            QVariantMap map;
            map["hour"] = slot.hour;
            map["showName"] = slot.showName;
            map["slug"] = slot.slug;
            map["subtitle"] = slot.subtitle;
            map["isRepeat"] = slot.isRepeat;
            map["isDito"] = slot.isDito;
            map["sendetippUrl"] = slot.sendetippUrl;
            return map;
        }
    }

    if (role == HourRole && !m_days.isEmpty() && row < m_days.first().entries.count()) {
        return m_days.first().entries[row].hour;
    }

    return QVariant();
}

QHash<int, QByteArray> ProgramModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[HourRole] = "hour";
    roles[Day0Role] = "day0";
    roles[Day1Role] = "day1";
    roles[Day2Role] = "day2";
    roles[Day3Role] = "day3";
    roles[Day4Role] = "day4";
    roles[Day5Role] = "day5";
    roles[Day6Role] = "day6";
    return roles;
}

int ProgramModel::weekOffset() const
{
    return m_weekOffset;
}

QString ProgramModel::weekLabel() const
{
    return m_weekLabel;
}

int ProgramModel::count() const
{
    if (m_days.isEmpty()) return 0;
    return m_days.first().entries.count();
}

void ProgramModel::loadFromHtml(const QString &html)
{
    beginResetModel();
    m_days = HtmlParser::parseProgramWeek(html, m_weekLabel);
    endResetModel();
    emit programChanged();
}

void ProgramModel::prevWeek()
{
    m_weekOffset--;
    emit weekOffsetChanged();
}

void ProgramModel::nextWeek()
{
    m_weekOffset++;
    emit weekOffsetChanged();
}

void ProgramModel::resetWeek()
{
    m_weekOffset = 0;
    emit weekOffsetChanged();
}