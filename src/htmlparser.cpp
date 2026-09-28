#include "htmlparser.h"
#include <QRegularExpression>

// ---------------------------------------------------------------------------
// decodeEntities – lightweight HTML entity decoder
// ---------------------------------------------------------------------------
QString HtmlParser::decodeEntities(const QString &text)
{
    QString result = text;
    result.replace("&amp;",  "&");
    result.replace("&lt;",   "<");
    result.replace("&gt;",   ">");
    result.replace("&quot;", "\"");
    result.replace("&#039;", "'");
    result.replace("&nbsp;", " ");
    return result.trimmed();
}

// ---------------------------------------------------------------------------
// parseProgramWeek
// ---------------------------------------------------------------------------
QList<ProgramDay> HtmlParser::parseProgramWeek(const QString &html, QString &weekLabel)
{
    QList<ProgramDay> days;
    weekLabel.clear();

    // 1. Week label
    QRegularExpression weekRe(
        QStringLiteral("<div\\s+class=\"program-week-header\">(.*?)</div>"),
        QRegularExpression::DotMatchesEverythingOption);
    QRegularExpressionMatch weekMatch = weekRe.match(html);
    if (weekMatch.hasMatch())
        weekLabel = decodeEntities(weekMatch.captured(1).trimmed());

    // 2. Day labels from thead <th class="day">
    QRegularExpression dayRe(
        QStringLiteral("<th\\s+class=\"day\">(.*?)</th>"),
        QRegularExpression::DotMatchesEverythingOption);
    QRegularExpressionMatchIterator dayIt = dayRe.globalMatch(html);
    QStringList dayLabels;
    while (dayIt.hasNext())
        dayLabels << decodeEntities(dayIt.next().captured(1).trimmed());

    if (dayLabels.size() != 7)
        return days;

    for (int d = 0; d < 7; ++d) {
        ProgramDay pd;
        pd.dayLabel = dayLabels[d];
        days << pd;
    }

    // 3. Extract tbody
    QRegularExpression tbodyRe(
        QStringLiteral("<tbody>(.*?)</tbody>"),
        QRegularExpression::DotMatchesEverythingOption);
    QRegularExpressionMatch tbodyMatch = tbodyRe.match(html);
    if (!tbodyMatch.hasMatch())
        return days;

    QString tbody = tbodyMatch.captured(1);

    // 4. Iterate over <tr> rows
    QRegularExpression trRe(
        QStringLiteral("<tr>(.*?)</tr>"),
        QRegularExpression::DotMatchesEverythingOption);
    QRegularExpressionMatchIterator trIt = trRe.globalMatch(tbody);

    // Regex for <td> cells (captures attrs + inner content)
    QRegularExpression tdRe(
        QStringLiteral("<td([^>]*)>(.*?)</td>"),
        QRegularExpression::DotMatchesEverythingOption);

    // Regex for <a> inside a cell
    QRegularExpression aRe(
        QStringLiteral("<a[^>]*href=\"([^\"]+)\"[^>]*>(.*?)</a>"),
        QRegularExpression::DotMatchesEverythingOption);

    // Regex for subtitle after <br/> or <br>
    QRegularExpression brRe(
        QStringLiteral("<br\\s*/?>\\s*(.+)$"),
        QRegularExpression::DotMatchesEverythingOption);

    // Regex for [Sendetipp: <a href="...">...</a>]
    QRegularExpression sendetippRe(
        QStringLiteral("\\[Sendetipp:.*?<a[^>]*href=\"([^\"]+)\""),
        QRegularExpression::DotMatchesEverythingOption);

    while (trIt.hasNext()) {
        QRegularExpressionMatch trMatch = trIt.next();
        QString rowHtml = trMatch.captured(1);

        // Collect all <td> cells in this row
        QList<QPair<QString, QString>> cells; // (attributes, innerHTML)
        QRegularExpressionMatchIterator tdIt = tdRe.globalMatch(rowHtml);
        while (tdIt.hasNext()) {
            QRegularExpressionMatch m = tdIt.next();
            cells << qMakePair(m.captured(1), m.captured(2));
        }

        if (cells.size() < 2)
            continue; // need at least hour + 1 day cell

        // First cell is the hour (time_show_week tablesaw-cell-persist)
        QString hourText = decodeEntities(cells.first().second);
        hourText.remove(QRegularExpression(QStringLiteral("[^0-9]")));
        bool ok;
        int hour = hourText.toInt(&ok);
        if (!ok)
            continue;

        // Remaining cells are the 7 day columns (index 1..7)
        for (int d = 0; d < 7 && (d + 1) < cells.size(); ++d) {
            ScheduleSlot slot;
            slot.hour = hour;

            const QString attrs = cells.at(d + 1).first;
            const QString content = cells.at(d + 1).second;

            // --- dito check ---
            if (attrs.contains(QStringLiteral("_dito"))) {
                slot.isDito = true;
                days[d].entries << slot;
                continue;
            }

            // --- repeat check ---
            if (content.contains(QStringLiteral("(Wdh.)")))
                slot.isRepeat = true;

            // --- <a> extraction (show name + slug) ---
            QRegularExpressionMatch aMatch = aRe.match(content);
            if (aMatch.hasMatch()) {
                QString href = aMatch.captured(1);
                slot.showName = decodeEntities(aMatch.captured(2));
                // derive slug from last path segment
                int lastSlash = href.lastIndexOf(QLatin1Char('/'));
                slot.slug = (lastSlash >= 0) ? href.mid(lastSlash + 1) : href;
            }

            // --- subtitle / sendetipp after <br/> ---
            QRegularExpressionMatch brMatch = brRe.match(content);
            if (brMatch.hasMatch()) {
                QString afterBr = brMatch.captured(1).trimmed();

                QRegularExpressionMatch sendetippMatch = sendetippRe.match(afterBr);
                if (sendetippMatch.hasMatch()) {
                    slot.sendetippUrl = sendetippMatch.captured(1);
                } else {
                    // Plain subtitle – strip residual HTML tags
                    QString sub = afterBr;
                    sub.remove(QRegularExpression(QStringLiteral("<[^>]+>")));
                    sub.remove(QStringLiteral("(Wdh.)"));
                    slot.subtitle = decodeEntities(sub);
                }
            }

            // --- sendetipp that might appear without preceding <br/> ---
            if (slot.sendetippUrl.isEmpty()) {
                QRegularExpressionMatch sendetippMatch2 = sendetippRe.match(content);
                if (sendetippMatch2.hasMatch())
                    slot.sendetippUrl = sendetippMatch2.captured(1);
            }

            days[d].entries << slot;
        }
    }

    return days;
}

// ---------------------------------------------------------------------------
// parseRecordings
// ---------------------------------------------------------------------------
QList<Recording> HtmlParser::parseRecordings(const QString &html)
{
    QList<Recording> recordings;

    // Split by <p class="plus7-day"> sections (the actual radiox.de structure)
    QRegularExpression daySectionRe(
        QStringLiteral("<p\\s+class=\"plus7-day\">(.*?)</p>"),
        QRegularExpression::DotMatchesEverythingOption);
    QRegularExpressionMatchIterator dayIt = daySectionRe.globalMatch(html);

    while (dayIt.hasNext()) {
        QRegularExpressionMatch dayMatch = dayIt.next();
        QString section = dayMatch.captured(1);

        // Extract date: "Montag, 28.09.2026" or "Monday, 28.09.2026"
        QRegularExpression dateRe(QStringLiteral("\\d{2}\\.\\d{2}\\.\\d{4}"));
        QRegularExpressionMatch dateMatch = dateRe.match(section);
        QDate sectionDate;
        if (dateMatch.hasMatch())
            sectionDate = QDate::fromString(dateMatch.captured(0), QStringLiteral("dd.MM.yyyy"));

        // Each recording block: time + <a onclick="plus7_show_recording(ID)">showName</a> + subtitle
        // Split on time patterns (HH:MM) to find each recording
        QRegularExpression recBlockRe(
            QStringLiteral("(\\d{2}:\\d{2})\\s*&nbsp;.*?plus7_show_recording\\((\\d+)\\).*?<\\/button>\\s*(.*?)(?=\\d{2}:\\d{2}|$)"),
            QRegularExpression::DotMatchesEverythingOption);
        QRegularExpressionMatchIterator recIt = recBlockRe.globalMatch(section);

        while (recIt.hasNext()) {
            QRegularExpressionMatch recMatch = recIt.next();
            Recording rec;
            rec.date = sectionDate;
            rec.time = QTime::fromString(recMatch.captured(1), QStringLiteral("HH:mm"));
            rec.id = recMatch.captured(2).toInt();
            rec.playbackUrl = QStringLiteral("/plus7/ajax/player/") + QString::number(rec.id);

            // Show name: extract from <a> tag in the captured block
            QString block = recMatch.captured(0);
            QRegularExpression showRe(
                QStringLiteral("<a[^>]*>\\s*([^<]+?)\\s*</a>"),
                QRegularExpression::DotMatchesEverythingOption);
            QRegularExpressionMatch showMatch = showRe.match(block);
            if (showMatch.hasMatch())
                rec.showName = decodeEntities(showMatch.captured(1));

            // Subtitle: text after </button> (captured group 3)
            QString afterButton = recMatch.captured(3).trimmed();
            // Strip any remaining HTML tags
            afterButton.remove(QRegularExpression(QStringLiteral("<[^>]+>")));
            afterButton = decodeEntities(afterButton);
            if (!afterButton.isEmpty())
                rec.subtitle = afterButton;

            recordings << rec;
        }
    }

    return recordings;
}

// ---------------------------------------------------------------------------
// parseSendetipps
// ---------------------------------------------------------------------------
QList<Sendetipp> HtmlParser::parseSendetipps(const QString &html)
{
    QList<Sendetipp> tipps;

    // Match each <article class="sendetipp"> ... </article>
    QRegularExpression articleRe(
        QStringLiteral("<article\\s+class=\"sendetipp\">(.*?)</article>"),
        QRegularExpression::DotMatchesEverythingOption);
    QRegularExpressionMatchIterator artIt = articleRe.globalMatch(html);

    while (artIt.hasNext()) {
        QRegularExpressionMatch artMatch = artIt.next();
        QString block = artMatch.captured(1);
        Sendetipp t;

        // Title from <h2><a href="...">...</a></h2>
        QRegularExpression titleRe(
            QStringLiteral("<h2>\\s*<a[^>]*href=\"([^\"]+)\"[^>]*>(.*?)</a>\\s*</h2>"),
            QRegularExpression::DotMatchesEverythingOption);
        QRegularExpressionMatch titleMatch = titleRe.match(block);
        if (titleMatch.hasMatch())
            t.title = decodeEntities(titleMatch.captured(2));

        // Show name + slug from <div class="sendetipp-show"><a href="...">...</a></div>
        QRegularExpression showRe(
            QStringLiteral("<div\\s+class=\"sendetipp-show\">\\s*<a[^>]*href=\"([^\"]+)\"[^>]*>(.*?)</a>"),
            QRegularExpression::DotMatchesEverythingOption);
        QRegularExpressionMatch showMatch = showRe.match(block);
        if (showMatch.hasMatch()) {
            t.showName = decodeEntities(showMatch.captured(2));
            QString href = showMatch.captured(1);
            int lastSlash = href.lastIndexOf(QLatin1Char('/'));
            t.showSlug = (lastSlash >= 0) ? href.mid(lastSlash + 1) : href;
        }

        // Date/time
        QRegularExpression metaRe(
            QStringLiteral("<div\\s+class=\"sendetipp-meta\">(.*?)</div>"),
            QRegularExpression::DotMatchesEverythingOption);
        QRegularExpressionMatch metaMatch = metaRe.match(block);
        if (metaMatch.hasMatch())
            t.dateTime = decodeEntities(metaMatch.captured(1));

        // Description
        QRegularExpression descRe(
            QStringLiteral("<div\\s+class=\"sendetipp-description\">(.*?)</div>"),
            QRegularExpression::DotMatchesEverythingOption);
        QRegularExpressionMatch descMatch = descRe.match(block);
        if (descMatch.hasMatch())
            t.description = decodeEntities(descMatch.captured(1));

        // Image
        QRegularExpression imgRe(
            QStringLiteral("<img\\s+class=\"sendetipp-image\"\\s+src=\"([^\"]+)\""),
            QRegularExpression::DotMatchesEverythingOption);
        QRegularExpressionMatch imgMatch = imgRe.match(block);
        if (imgMatch.hasMatch())
            t.imageUrl = imgMatch.captured(1);

        tipps << t;
    }

    return tipps;
}

// ---------------------------------------------------------------------------
// parseShowDetail – placeholder for future implementation
// ---------------------------------------------------------------------------
QVariantMap HtmlParser::parseShowDetail(const QString &html)
{
    Q_UNUSED(html);
    return {};
}