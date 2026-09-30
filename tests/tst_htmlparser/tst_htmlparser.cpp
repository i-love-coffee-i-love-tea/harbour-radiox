#include <QtTest>
#include <QFile>
#include <QCoreApplication>
#include "htmlparser.h"

class TestHtmlParser : public QObject
{
    Q_OBJECT

private:
    QString m_programWeekHtml;
    QString m_plus7Html;
    QString m_sendetippsHtml;
    QString m_showDetailHtml;

    QString loadFixture(const QString &name)
    {
        QString path = QString("%1/fixtures/%2")
            .arg(QCoreApplication::applicationDirPath(), name);
        // Try relative to source dir when running from build dir
        QFile f(path);
        if (!f.exists()) {
            path = QString("%1/tests/fixtures/%2")
                .arg(QDir::currentPath(), name);
            f.setFileName(path);
        }
        if (!f.exists()) {
            // Try from the test source directory
            path = QString(SRCDIR "/fixtures/%2").arg(name);
            f.setFileName(path);
        }
        if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
            return QString();
        return QString::fromUtf8(f.readAll());
    }

private slots:
    void initTestCase()
    {
        m_programWeekHtml = loadFixture("program_week.html");
        m_plus7Html = loadFixture("plus7_content_all.html");
        m_sendetippsHtml = loadFixture("sendetipps.html");
        m_showDetailHtml = loadFixture("show_detail.html");
        QVERIFY2(!m_programWeekHtml.isEmpty(), "Failed to load program_week.html fixture");
        QVERIFY2(!m_plus7Html.isEmpty(), "Failed to load plus7_content_all.html fixture");
        QVERIFY2(!m_sendetippsHtml.isEmpty(), "Failed to load sendetipps.html fixture");
        QVERIFY2(!m_showDetailHtml.isEmpty(), "Failed to load show_detail.html fixture");
    }

    // --- parseProgramWeek tests ---

    void testParseProgramWeek_weekLabel()
    {
        QString weekLabel;
        HtmlParser::parseProgramWeek(m_programWeekHtml, weekLabel);
        QVERIFY(weekLabel.contains("KW 40"));
    }

    void testParseProgramWeek_dayCount()
    {
        QString weekLabel;
        QList<ProgramDay> days = HtmlParser::parseProgramWeek(m_programWeekHtml, weekLabel);
        QCOMPARE(days.size(), 7);
    }

    void testParseProgramWeek_slotCount()
    {
        QString weekLabel;
        QList<ProgramDay> days = HtmlParser::parseProgramWeek(m_programWeekHtml, weekLabel);
        for (const ProgramDay &day : days) {
            QCOMPARE(day.entries.size(), 3);
        }
    }

    void testParseProgramWeek_showName()
    {
        QString weekLabel;
        QList<ProgramDay> days = HtmlParser::parseProgramWeek(m_programWeekHtml, weekLabel);
        QVERIFY(!days.isEmpty());
        QCOMPARE(days[0].entries[0].showName, QString("Sounds & Tapes"));
    }

    void testParseProgramWeek_slug()
    {
        QString weekLabel;
        QList<ProgramDay> days = HtmlParser::parseProgramWeek(m_programWeekHtml, weekLabel);
        QVERIFY(!days.isEmpty());
        QCOMPARE(days[0].entries[0].slug, QString("sounds-and-tapes"));
    }

    void testParseProgramWeek_dito()
    {
        QString weekLabel;
        QList<ProgramDay> days = HtmlParser::parseProgramWeek(m_programWeekHtml, weekLabel);
        // Row 2 (hour 07): days 5 and 6 are dito
        QCOMPARE(days[5].entries[1].isDito, true);
        QCOMPARE(days[6].entries[1].isDito, true);
        // day 0 is not dito
        QCOMPARE(days[0].entries[1].isDito, false);
    }

    void testParseProgramWeek_repeat()
    {
        QString weekLabel;
        QList<ProgramDay> days = HtmlParser::parseProgramWeek(m_programWeekHtml, weekLabel);
        // Row 3 (hour 17): day 2 has (Wdh.)
        QCOMPARE(days[2].entries[2].isRepeat, true);
        QCOMPARE(days[5].entries[2].isRepeat, true);
        // day 0 hour 17 is not repeat
        QCOMPARE(days[0].entries[2].isRepeat, false);
    }

    void testParseProgramWeek_subtitle()
    {
        QString weekLabel;
        QList<ProgramDay> days = HtmlParser::parseProgramWeek(m_programWeekHtml, weekLabel);
        // Row 3 (hour 17), day 0: subtitle "Special: Jazz Night"
        QCOMPARE(days[0].entries[2].subtitle, QString("Special: Jazz Night"));
        // Row 3 (hour 17), day 4: subtitle "Linux Kernel Deep Dive"
        QCOMPARE(days[4].entries[2].subtitle, QString("Linux Kernel Deep Dive"));
    }

    void testParseProgramWeek_sendetipp()
    {
        QString weekLabel;
        QList<ProgramDay> days = HtmlParser::parseProgramWeek(m_programWeekHtml, weekLabel);
        // Row 3 (hour 17), day 1: sendetipp URL
        QCOMPARE(days[1].entries[2].sendetippUrl, QString("/sendetipps/ton-art-special"));
    }

    void testParseProgramWeek_ampersand()
    {
        QString weekLabel;
        QList<ProgramDay> days = HtmlParser::parseProgramWeek(m_programWeekHtml, weekLabel);
        // All "Sounds &amp; Tapes" should be decoded to "Sounds & Tapes"
        QCOMPARE(days[0].entries[0].showName, QString("Sounds & Tapes"));
        QCOMPARE(days[1].entries[0].showName, QString("Sounds & Tapes"));
        QCOMPARE(days[2].entries[0].showName, QString("Sounds & Tapes"));
    }

    // --- parseRecordings tests ---

    void testParseRecordings_count()
    {
        QList<Recording> recs = HtmlParser::parseRecordings(m_plus7Html);
        QCOMPARE(recs.size(), 3);
    }

    void testParseRecordings_fields()
    {
        QList<Recording> recs = HtmlParser::parseRecordings(m_plus7Html);
        QCOMPARE(recs[0].id, 48867);
        QCOMPARE(recs[0].showName, QString("scrambled x"));
        QCOMPARE(recs[0].subtitle, QString("mit Nicky und Thomas"));

        QCOMPARE(recs[1].id, 48854);
        QCOMPARE(recs[1].showName, QString("VirusMusikRadio"));
        QCOMPARE(recs[1].subtitle, QString("Szene Sachsenhausen mit Cordyonbass"));

        QCOMPARE(recs[2].id, 48849);
        QCOMPARE(recs[2].showName, QString("SISU-Radio"));
        QCOMPARE(recs[2].subtitle, QString("Viela kesaisissa tunnelmissa"));
    }

    void testParseRecordings_playbackUrl()
    {
        QList<Recording> recs = HtmlParser::parseRecordings(m_plus7Html);
        QVERIFY(recs[0].playbackUrl.contains("/plus7/ajax/player/48867"));
    }

    // --- parseSendetipps tests ---

    void testParseSendetipps_count()
    {
        QList<Sendetipp> tipps = HtmlParser::parseSendetipps(m_sendetippsHtml);
        QCOMPARE(tipps.size(), 2);
    }

    void testParseSendetipps_fields()
    {
        QList<Sendetipp> tipps = HtmlParser::parseSendetipps(m_sendetippsHtml);
        QVERIFY(!tipps[0].title.isEmpty());
        QVERIFY(!tipps[0].showName.isEmpty());
        QVERIFY(!tipps[0].showSlug.isEmpty());

        QVERIFY(!tipps[1].title.isEmpty());
        QVERIFY(!tipps[1].showName.isEmpty());
        QVERIFY(!tipps[1].showSlug.isEmpty());
    }

    // --- parseShowDetail tests ---

    void testParseShowDetail_title()
    {
        QVariantMap detail = HtmlParser::parseShowDetail(m_showDetailHtml);
        QCOMPARE(detail["title"].toString(), QStringLiteral("Sounds & Tapes"));
    }

    void testParseShowDetail_description()
    {
        QVariantMap detail = HtmlParser::parseShowDetail(m_showDetailHtml);
        QString desc = detail["description"].toString();
        QVERIFY(desc.contains("experimental music"));
        QVERIFY(desc.contains("Frankfurt"));
    }

    void testParseShowDetail_imageUrl()
    {
        QVariantMap detail = HtmlParser::parseShowDetail(m_showDetailHtml);
        QString imgUrl = detail["imageUrl"].toString();
        QVERIFY(imgUrl.contains("sounds-and-tapes.jpg"));
    }

    void testParseShowDetail_relativeImageUrl()
    {
        // Relative image URL should be prefixed with base URL
        QVariantMap detail = HtmlParser::parseShowDetail(m_showDetailHtml);
        QString imgUrl = detail["imageUrl"].toString();
        QVERIFY(imgUrl.startsWith("http"));
    }

    void testParseShowDetail_emptyHtml()
    {
        QVariantMap detail = HtmlParser::parseShowDetail(QString());
        QVERIFY(detail.isEmpty());
    }

    void testParseShowDetail_missingFields()
    {
        // Minimal HTML with no article body
        QString html = QStringLiteral("<html><body><h1 itemprop=\"name\">Test</h1></body></html>");
        QVariantMap detail = HtmlParser::parseShowDetail(html);
        QCOMPARE(detail["title"].toString(), QStringLiteral("Test"));
        QVERIFY(!detail.contains("description"));
        QVERIFY(!detail.contains("imageUrl"));
    }
};

QTEST_MAIN(TestHtmlParser)
#include "tst_htmlparser.moc"