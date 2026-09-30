#ifndef RECORDINGSMODEL_H
#define RECORDINGSMODEL_H

#include <QAbstractListModel>
#include "htmlparser.h"

class RecordingsModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Roles {
        IdRole = Qt::UserRole + 1,
        DateRole, TimeRole, ShowNameRole,
        SubtitleRole, PlaybackUrlRole, DayHeaderRole
    };

    explicit RecordingsModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void loadFromHtml(const QString &html);

signals:
    void recordingsChanged();

private:
    QList<Recording> m_recordings;
    QList<int> m_dayHeaderIndices; // indices where a new day starts
};

#endif // RECORDINGSMODEL_H