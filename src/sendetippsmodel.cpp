#include "sendetippsmodel.h"

SendetippsModel::SendetippsModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int SendetippsModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;
    return m_tipps.count();
}

QVariant SendetippsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_tipps.count())
        return QVariant();

    const Sendetipp &t = m_tipps[index.row()];

    switch (role) {
    case TitleRole: return t.title;
    case ShowNameRole: return t.showName;
    case ShowSlugRole: return t.showSlug;
    case DateTimeRole: return t.dateTime;
    case DescriptionRole: return t.description;
    case ImageUrlRole: return t.imageUrl;
    }

    return QVariant();
}

QHash<int, QByteArray> SendetippsModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[TitleRole] = "title";
    roles[ShowNameRole] = "showName";
    roles[ShowSlugRole] = "showSlug";
    roles[DateTimeRole] = "dateTime";
    roles[DescriptionRole] = "description";
    roles[ImageUrlRole] = "imageUrl";
    return roles;
}

void SendetippsModel::loadFromHtml(const QString &html)
{
    beginResetModel();
    m_tipps = HtmlParser::parseSendetipps(html);
    endResetModel();
    emit sendetippsChanged();
}