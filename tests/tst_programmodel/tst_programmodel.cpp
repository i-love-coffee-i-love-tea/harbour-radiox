#include <QtTest>
#include <QAbstractListModel>
#include "programmodel.h"

class ProgramModelTest : public QObject
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
    void init()
    {
        // Reset global state if needed
    }

    void testLoadFromHtml_rowCount()
    {
        ProgramModel model;
        model.loadFromHtml(loadFixture("program_week.html"));
        QCOMPARE(model.rowCount(), 3); // 3 rows in fixture
    }

    void testLoadFromHtml_weekLabel()
    {
        ProgramModel model;
        model.loadFromHtml(loadFixture("program_week.html"));
        QVERIFY(model.weekLabel().contains("KW 40"));
    }

    void testDay0Role_showName()
    {
        ProgramModel model;
        model.loadFromHtml(loadFixture("program_week.html"));
        QModelIndex idx = model.index(0);
        QVariantMap day0 = model.data(idx, ProgramModel::Day0Role).toMap();
        QCOMPARE(day0["showName"].toString(), QStringLiteral("Sounds & Tapes"));
    }

    void testDay0Role_slug()
    {
        ProgramModel model;
        model.loadFromHtml(loadFixture("program_week.html"));
        QModelIndex idx = model.index(0);
        QVariantMap day0 = model.data(idx, ProgramModel::Day0Role).toMap();
        QCOMPARE(day0["slug"].toString(), QStringLiteral("sounds-and-tapes"));
    }

    void testDitoCells()
    {
        ProgramModel model;
        model.loadFromHtml(loadFixture("program_week.html"));
        QModelIndex idx = model.index(1); // hour 07 row
        // Days 5 and 6 (Sa, So) should be dito
        QVariantMap day5 = model.data(idx, ProgramModel::Day5Role).toMap();
        QVariantMap day6 = model.data(idx, ProgramModel::Day6Role).toMap();
        QCOMPARE(day5["isDito"].toBool(), true);
        QCOMPARE(day6["isDito"].toBool(), true);
        // Day 0 should not be dito
        QVariantMap day0 = model.data(idx, ProgramModel::Day0Role).toMap();
        QCOMPARE(day0["isDito"].toBool(), false);
    }

    void testWeekOffset()
    {
        ProgramModel model;
        QCOMPARE(model.weekOffset(), 0);
        model.prevWeek();
        QCOMPARE(model.weekOffset(), -1);
        model.nextWeek();
        QCOMPARE(model.weekOffset(), 0);
        model.nextWeek();
        QCOMPARE(model.weekOffset(), 1);
        model.resetWeek();
        QCOMPARE(model.weekOffset(), 0);
    }

    void testRoleNames()
    {
        ProgramModel model;
        QHash<int, QByteArray> roles = model.roleNames();
        QCOMPARE(roles[ProgramModel::HourRole], QByteArray("hour"));
        QCOMPARE(roles[ProgramModel::Day0Role], QByteArray("day0"));
        QCOMPARE(roles[ProgramModel::Day6Role], QByteArray("day6"));
    }

    void testEmptyModel()
    {
        ProgramModel model;
        QCOMPARE(model.rowCount(), 0);
        QCOMPARE(model.weekLabel(), QString());
        QCOMPARE(model.data(model.index(0), ProgramModel::HourRole), QVariant());
    }
};

QTEST_MAIN(ProgramModelTest)
#include "tst_programmodel.moc"