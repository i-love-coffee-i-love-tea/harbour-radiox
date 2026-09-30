#include <QtTest>
#include "sendetippsmodel.h"

class SendetippsModelTest : public QObject
{
    Q_OBJECT
private:
    QString loadFixture(const QString &name)
    {
        QFile f(QStringLiteral(SRCDIR "/fixtures/") + name);
        if (!f.open(QIODevice::ReadOnly)) return QString();
        return QString::fromUtf8(f.readAll());
    }

private slots:
    void testRowCount()
    {
        SendetippsModel model;
        model.loadFromHtml(loadFixture("sendetipps.html"));
        QCOMPARE(model.rowCount(), 2);
    }

    void testTitle()
    {
        SendetippsModel model;
        model.loadFromHtml(loadFixture("sendetipps.html"));
        QModelIndex idx = model.index(0);
        QCOMPARE(model.data(idx, SendetippsModel::TitleRole).toString(),
                 QStringLiteral("Ton Art - Special Edition"));
    }

    void testShowName()
    {
        SendetippsModel model;
        model.loadFromHtml(loadFixture("sendetipps.html"));
        QModelIndex idx = model.index(0);
        QCOMPARE(model.data(idx, SendetippsModel::ShowNameRole).toString(),
                 QStringLiteral("Ton Art"));
    }

    void testShowSlug()
    {
        SendetippsModel model;
        model.loadFromHtml(loadFixture("sendetipps.html"));
        QModelIndex idx = model.index(0);
        QCOMPARE(model.data(idx, SendetippsModel::ShowSlugRole).toString(),
                 QStringLiteral("ton-art"));
    }

    void testDateTime()
    {
        SendetippsModel model;
        model.loadFromHtml(loadFixture("sendetipps.html"));
        QModelIndex idx = model.index(0);
        QCOMPARE(model.data(idx, SendetippsModel::DateTimeRole).toString(),
                 QStringLiteral("Do, 01.10.2026, 17:00"));
    }

    void testDescription()
    {
        SendetippsModel model;
        model.loadFromHtml(loadFixture("sendetipps.html"));
        QModelIndex idx = model.index(0);
        QCOMPARE(model.data(idx, SendetippsModel::DescriptionRole).toString(),
                 QStringLiteral("Eine besondere Ausgabe der Ton Art mit experimentellen Klangcollagen."));
    }

    void testImageUrl()
    {
        SendetippsModel model;
        model.loadFromHtml(loadFixture("sendetipps.html"));
        QModelIndex idx = model.index(0);
        QCOMPARE(model.data(idx, SendetippsModel::ImageUrlRole).toString(),
                 QStringLiteral("/img/sendetipps/ton-art-special.jpg"));
    }

    void testSecondItem()
    {
        SendetippsModel model;
        model.loadFromHtml(loadFixture("sendetipps.html"));
        QModelIndex idx = model.index(1);
        QCOMPARE(model.data(idx, SendetippsModel::TitleRole).toString(),
                 QStringLiteral("Blue Danube Radio - Live Session"));
        QCOMPARE(model.data(idx, SendetippsModel::ShowSlugRole).toString(),
                 QStringLiteral("blue-danube-radio"));
    }

    void testRoleNames()
    {
        SendetippsModel model;
        QHash<int, QByteArray> roles = model.roleNames();
        QCOMPARE(roles[SendetippsModel::TitleRole], QByteArray("title"));
        QCOMPARE(roles[SendetippsModel::ShowNameRole], QByteArray("showName"));
        QCOMPARE(roles[SendetippsModel::ImageUrlRole], QByteArray("imageUrl"));
    }

    void testEmptyModel()
    {
        SendetippsModel model;
        QCOMPARE(model.rowCount(), 0);
    }
};

QTEST_MAIN(SendetippsModelTest)
#include "tst_sendetippsmodel.moc"