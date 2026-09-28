#include <QtTest>
#include "recordingsmodel.h"

class RecordingsModelTest : public QObject
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
        RecordingsModel model;
        model.loadFromHtml(loadFixture("plus7_content_all.html"));
        QCOMPARE(model.rowCount(), 3);
    }

    void testShowName()
    {
        RecordingsModel model;
        model.loadFromHtml(loadFixture("plus7_content_all.html"));
        QModelIndex idx = model.index(0);
        QCOMPARE(model.data(idx, RecordingsModel::ShowNameRole).toString(),
                 QStringLiteral("scrambled x"));
    }

    void testRecordingId()
    {
        RecordingsModel model;
        model.loadFromHtml(loadFixture("plus7_content_all.html"));
        QModelIndex idx = model.index(0);
        QCOMPARE(model.data(idx, RecordingsModel::IdRole).toInt(), 48867);
    }

    void testPlaybackUrl()
    {
        RecordingsModel model;
        model.loadFromHtml(loadFixture("plus7_content_all.html"));
        QModelIndex idx = model.index(0);
        QString url = model.data(idx, RecordingsModel::PlaybackUrlRole).toString();
        QVERIFY(url.contains("/plus7/ajax/player/48867"));
    }

    void testSubtitle()
    {
        RecordingsModel model;
        model.loadFromHtml(loadFixture("plus7_content_all.html"));
        QModelIndex idx = model.index(0);
        QString sub = model.data(idx, RecordingsModel::SubtitleRole).toString();
        QVERIFY(sub.contains("Nicky und Thomas"));
    }

    void testDayHeader()
    {
        RecordingsModel model;
        model.loadFromHtml(loadFixture("plus7_content_all.html"));
        // First item of first day group should have a day header
        QString header = model.data(model.index(0), RecordingsModel::DayHeaderRole).toString();
        QVERIFY(!header.isEmpty());
    }

    void testRoleNames()
    {
        RecordingsModel model;
        QHash<int, QByteArray> roles = model.roleNames();
        QCOMPARE(roles[RecordingsModel::IdRole], QByteArray("recordingId"));
        QCOMPARE(roles[RecordingsModel::ShowNameRole], QByteArray("showName"));
    }

    void testEmptyModel()
    {
        RecordingsModel model;
        QCOMPARE(model.rowCount(), 0);
    }
};

QTEST_MAIN(RecordingsModelTest)
#include "tst_recordingsmodel.moc"