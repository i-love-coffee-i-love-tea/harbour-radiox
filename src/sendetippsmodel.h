#ifndef SENDETIPPSMODEL_H
#define SENDETIPPSMODEL_H

#include <QAbstractListModel>
#include "htmlparser.h"

class SendetippsModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Roles {
        TitleRole = Qt::UserRole + 1,
        ShowNameRole, ShowSlugRole,
        DateTimeRole, DescriptionRole, ImageUrlRole
    };

    explicit SendetippsModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void loadFromHtml(const QString &html);

signals:
    void sendetippsChanged();

private:
    QList<Sendetipp> m_tipps;
};

#endif // SENDETIPPSMODEL_H