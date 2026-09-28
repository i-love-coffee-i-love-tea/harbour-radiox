#include "recordingsmodel.h"

RecordingsModel::RecordingsModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int RecordingsModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;
    return m_recordings.count();
}

QVariant RecordingsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_recordings.count())
        return QVariant();

    const Recording &r = m_recordings[index.row()];

    switch (role) {
    case IdRole: return r.id;
    case DateRole: return r.date;
    case TimeRole: return r.time;
    case ShowNameRole: return r.showName;
    case SubtitleRole: return r.subtitle;
    case PlaybackUrlRole: return r.playbackUrl;
    case DayHeaderRole:
        if (m_dayHeaderIndices.contains(index.row()))
            return r.date.toString("dddd, dd.MM.yyyy");
        return QString();
    }

    return QVariant();
}

QHash<int, QByteArray> RecordingsModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[IdRole] = "recordingId";
    roles[DateRole] = "date";
    roles[TimeRole] = "time";
    roles[ShowNameRole] = "showName";
    roles[SubtitleRole] = "subtitle";
    roles[PlaybackUrlRole] = "playbackUrl";
    roles[DayHeaderRole] = "dayHeader";
    return roles;
}

void RecordingsModel::loadFromHtml(const QString &html)
{
    beginResetModel();
    m_recordings = HtmlParser::parseRecordings(html);
    m_dayHeaderIndices.clear();
    QDate lastDate;
    for (int i = 0; i < m_recordings.size(); ++i) {
        if (m_recordings[i].date != lastDate) {
            m_dayHeaderIndices.append(i);
            lastDate = m_recordings[i].date;
        }
    }
    endResetModel();
    emit recordingsChanged();
}