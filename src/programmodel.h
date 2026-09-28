#ifndef PROGRAMMODEL_H
#define PROGRAMMODEL_H

#include <QAbstractListModel>
#include "htmlparser.h"

class ProgramModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int weekOffset READ weekOffset NOTIFY weekOffsetChanged)
    Q_PROPERTY(QString weekLabel READ weekLabel NOTIFY programChanged)
    Q_PROPERTY(int count READ count NOTIFY programChanged)
public:
    enum Roles {
        HourRole = Qt::UserRole + 1,
        Day0Role, Day1Role, Day2Role, Day3Role,
        Day4Role, Day5Role, Day6Role
    };

    explicit ProgramModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    int weekOffset() const;
    QString weekLabel() const;
    int count() const;

    void loadFromHtml(const QString &html);

public slots:
    void prevWeek();
    void nextWeek();
    void resetWeek();

signals:
    void weekOffsetChanged();
    void programChanged();

private:
    QList<ProgramDay> m_days;
    QString m_weekLabel;
    int m_weekOffset = 0;
};

#endif // PROGRAMMODEL_H