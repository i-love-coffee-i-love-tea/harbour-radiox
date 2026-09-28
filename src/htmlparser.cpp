#include "htmlparser.h"
#include <libxml/HTMLparser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include <QRegularExpression>

// ---------------------------------------------------------------------------
// libxml2 helpers
// ---------------------------------------------------------------------------

static QString nodeText(xmlNodePtr node)
{
    if (!node) return QString();
    xmlChar *content = xmlNodeGetContent(node);
    if (!content) return QString();
    QString result = QString::fromUtf8(reinterpret_cast<const char*>(content));
    xmlFree(content);
    return result.trimmed();
}

static QString nodeAttr(xmlNodePtr node, const char *name)
{
    if (!node) return QString();
    xmlChar *val = xmlGetProp(node, BAD_CAST name);
    if (!val) return QString();
    QString result = QString::fromUtf8(reinterpret_cast<const char*>(val));
    xmlFree(val);
    return result;
}

static xmlXPathObjectPtr xpathEval(xmlDocPtr doc, xmlNodePtr ctxNode, const char *expr)
{
    xmlXPathContextPtr ctx = xmlXPathNewContext(doc);
    if (!ctx) return nullptr;
    ctx->node = ctxNode;
    xmlXPathObjectPtr result = xmlXPathEvalExpression(BAD_CAST expr, ctx);
    xmlXPathFreeContext(ctx);
    return result;
}

static int xpathNodeCount(xmlXPathObjectPtr obj)
{
    if (!obj || !obj->nodesetval) return 0;
    return obj->nodesetval->nodeNr;
}

static xmlNodePtr xpathNode(xmlXPathObjectPtr obj, int index)
{
    if (!obj || !obj->nodesetval) return nullptr;
    if (index < 0 || index >= obj->nodesetval->nodeNr) return nullptr;
    return obj->nodesetval->nodeTab[index];
}

// Walk previous siblings to find text containing a time pattern (HH:MM)
static QString findTimeBefore(xmlNodePtr node)
{
    for (xmlNodePtr cur = node->prev; cur; cur = cur->prev) {
        if (cur->type == XML_TEXT_NODE) {
            QString text = QString::fromUtf8(reinterpret_cast<const char*>(cur->content));
            QRegularExpression re(QStringLiteral("(\\d{2}:\\d{2})"));
            QRegularExpressionMatch m = re.match(text);
            if (m.hasMatch())
                return m.captured(1);
        }
    }
    return QString();
}

// Walk next siblings after a node to find the next element of a given tag
static xmlNodePtr findNextElement(xmlNodePtr node, const char *tag)
{
    for (xmlNodePtr cur = node->next; cur; cur = cur->next) {
        if (cur->type == XML_ELEMENT_NODE &&
            xmlStrcasecmp(cur->name, BAD_CAST tag) == 0)
            return cur;
    }
    return nullptr;
}

// Walk next siblings after a node to collect text until the next element
static QString textAfterElement(xmlNodePtr element)
{
    QString result;
    for (xmlNodePtr cur = element->next; cur; cur = cur->next) {
        if (cur->type == XML_TEXT_NODE) {
            result += QString::fromUtf8(reinterpret_cast<const char*>(cur->content));
        } else if (cur->type == XML_ELEMENT_NODE) {
            break;
        }
    }
    return result.trimmed();
}

static xmlDocPtr parseHtml(const QString &html)
{
    QByteArray utf8 = html.toUtf8();
    return htmlReadDoc(
        BAD_CAST utf8.constData(),
        nullptr, // URL
        "UTF-8",
        HTML_PARSE_RECOVER | HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING | HTML_PARSE_NONET
    );
}

// ---------------------------------------------------------------------------
// parseProgramWeek
// ---------------------------------------------------------------------------
QList<ProgramDay> HtmlParser::parseProgramWeek(const QString &html, QString &weekLabel)
{
    QList<ProgramDay> days;
    weekLabel.clear();

    xmlDocPtr doc = parseHtml(html);
    if (!doc) return days;

    // 1. Week label
    xmlXPathObjectPtr weekObj = xpathEval(doc, nullptr,
        "//div[@class='program-week-header']/p");
    if (weekObj) {
        xmlNodePtr weekNode = xpathNode(weekObj, 0);
        if (weekNode)
            weekLabel = nodeText(weekNode);
        xmlXPathFreeObject(weekObj);
    }

    // 2. Day labels
    xmlXPathObjectPtr dayObj = xpathEval(doc, nullptr, "//th[@class='day']");
    if (!dayObj) { xmlFreeDoc(doc); return days; }

    int dayCount = xpathNodeCount(dayObj);
    if (dayCount != 7) {
        xmlXPathFreeObject(dayObj);
        xmlFreeDoc(doc);
        return days;
    }

    for (int d = 0; d < 7; ++d) {
        ProgramDay pd;
        pd.dayLabel = nodeText(xpathNode(dayObj, d));
        days << pd;
    }
    xmlXPathFreeObject(dayObj);

    // 3. Iterate rows
    xmlXPathObjectPtr rowObj = xpathEval(doc, nullptr,
        "//table[@id='weektable']//tbody/tr[@class='tbodytr']");
    if (!rowObj) { xmlFreeDoc(doc); return days; }

    int rowCount = xpathNodeCount(rowObj);

    for (int r = 0; r < rowCount; ++r) {
        xmlNodePtr rowNode = xpathNode(rowObj, r);

        // Hour from first time cell
        xmlXPathObjectPtr hourObj = xpathEval(doc, rowNode,
            "td[contains(@class,'time_show_week')]");
        if (!hourObj) continue;

        int hourCount = xpathNodeCount(hourObj);
        if (hourCount < 1) { xmlXPathFreeObject(hourObj); continue; }

        QString hourStr = nodeText(xpathNode(hourObj, 0));
        bool ok;
        int hour = hourStr.toInt(&ok);
        xmlXPathFreeObject(hourObj);
        if (!ok) continue;

        // Day cells (cells with class containing 'hour_')
        xmlXPathObjectPtr cellObj = xpathEval(doc, rowNode,
            "td[contains(@class,'hour_')]");
        if (!cellObj) continue;

        int cellCount = xpathNodeCount(cellObj);

        for (int d = 0; d < 7 && d < cellCount; ++d) {
            xmlNodePtr cellNode = xpathNode(cellObj, d);
            ScheduleSlot slot;
            slot.hour = hour;

            QString cellClass = nodeAttr(cellNode, "class");

            // Dito check
            if (cellClass.contains(QStringLiteral("_dito"))) {
                slot.isDito = true;
                days[d].entries << slot;
                continue;
            }

            // Get show_week_hour div content
            xmlXPathObjectPtr divObj = xpathEval(doc, cellNode,
                "div[@class='show_week_hour']");
            if (!divObj || xpathNodeCount(divObj) == 0) {
                if (divObj) xmlXPathFreeObject(divObj);
                days[d].entries << slot;
                continue;
            }
            xmlNodePtr divNode = xpathNode(divObj, 0);

            // Full text of the div (for repeat/sendetipp checks)
            QString fullText = nodeText(divNode);

            // Show name from <a>
            xmlXPathObjectPtr aObj = xpathEval(doc, divNode, "a[1]");
            if (aObj && xpathNodeCount(aObj) > 0) {
                xmlNodePtr aNode = xpathNode(aObj, 0);
                slot.showName = nodeText(aNode);
                // Slug from href
                QString href = nodeAttr(aNode, "href");
                int lastSlash = href.lastIndexOf(QLatin1Char('/'));
                slot.slug = (lastSlash >= 0) ? href.mid(lastSlash + 1) : href;
                xmlXPathFreeObject(aObj);
            } else {
                if (aObj) xmlXPathFreeObject(aObj);
                // No link — plain text show name (e.g. "Musikmix")
                slot.showName = fullText;
                // Remove any (Wdh.) or [Sendetipp...] from it
                slot.showName.remove(QStringLiteral("(Wdh.)"));
                slot.showName.remove(QRegularExpression(QStringLiteral("\\[.*?Sendetipp.*?\\]")));
                slot.showName = slot.showName.trimmed();
            }

            // Repeat check
            slot.isRepeat = fullText.contains(QStringLiteral("(Wdh.)"));

            // Subtitle: text after <br/> in the div
            xmlNodePtr brNode = findNextElement(divNode->children, "br");
            if (brNode) {
                QString afterBr = textAfterElement(brNode);
                // Strip (Wdh.) and [Sendetipp...] markup
                afterBr.remove(QStringLiteral("(Wdh.)"));
                afterBr.remove(QRegularExpression(QStringLiteral("\\[.*?Sendetipp.*?\\]")));
                slot.subtitle = afterBr.trimmed();
            }

            // Sendetipp link
            if (fullText.contains(QStringLiteral("Sendetipp"))) {
                xmlXPathObjectPtr stObj = xpathEval(doc, divNode,
                    ".//a[contains(@href,'sendetipps')]");
                if (stObj && xpathNodeCount(stObj) > 0) {
                    slot.sendetippUrl = nodeAttr(xpathNode(stObj, 0), "href");
                    xmlXPathFreeObject(stObj);
                } else if (stObj) {
                    xmlXPathFreeObject(stObj);
                }
            }

            xmlXPathFreeObject(divObj);
            days[d].entries << slot;
        }

        xmlXPathFreeObject(cellObj);
    }

    xmlXPathFreeObject(rowObj);
    xmlFreeDoc(doc);
    return days;
}

// ---------------------------------------------------------------------------
// parseRecordings
// ---------------------------------------------------------------------------
QList<Recording> HtmlParser::parseRecordings(const QString &html)
{
    QList<Recording> recordings;

    xmlDocPtr doc = parseHtml(html);
    if (!doc) return recordings;

    // Each <p class="plus7-day"> is a day section
    xmlXPathObjectPtr sectionObj = xpathEval(doc, nullptr,
        "//p[contains(@class,'plus7-day')]");
    if (!sectionObj) { xmlFreeDoc(doc); return recordings; }

    int sectionCount = xpathNodeCount(sectionObj);

    for (int s = 0; s < sectionCount; ++s) {
        xmlNodePtr sectionNode = xpathNode(sectionObj, s);

        // Extract date from text content: "Montag, 28.09.2026"
        QString sectionText = nodeText(sectionNode);
        QRegularExpression dateRe(QStringLiteral("(\\d{2}\\.\\d{2}\\.\\d{4})"));
        QRegularExpressionMatch dateMatch = dateRe.match(sectionText);
        QDate sectionDate;
        if (dateMatch.hasMatch())
            sectionDate = QDate::fromString(dateMatch.captured(1), QStringLiteral("dd.MM.yyyy"));

        // Find all recording links within this section
        xmlXPathObjectPtr linkObj = xpathEval(doc, sectionNode,
            ".//a[contains(@onclick,'plus7_show_recording')]");
        if (!linkObj) continue;

        int linkCount = xpathNodeCount(linkObj);

        for (int i = 0; i < linkCount; ++i) {
            xmlNodePtr aNode = xpathNode(linkObj, i);
            Recording rec;
            rec.date = sectionDate;

            // Show name
            rec.showName = nodeText(aNode);

            // Recording ID from onclick
            QString onclick = nodeAttr(aNode, "onclick");
            QRegularExpression idRe(QStringLiteral("plus7_show_recording\\((\\d+)\\)"));
            QRegularExpressionMatch idMatch = idRe.match(onclick);
            if (idMatch.hasMatch()) {
                rec.id = idMatch.captured(1).toInt();
                rec.playbackUrl = QStringLiteral("/plus7/ajax/player/") + QString::number(rec.id);
            }

            // Time: look backwards in preceding text nodes
            QString timeStr = findTimeBefore(aNode);
            rec.time = QTime::fromString(timeStr, QStringLiteral("HH:mm"));

            // Subtitle: text after the next <button> sibling
            xmlNodePtr btnNode = findNextElement(aNode->next, "button");
            if (btnNode) {
                QString sub = textAfterElement(btnNode);
                // Remove HTML entities artifacts
                sub = sub.trimmed();
                if (!sub.isEmpty())
                    rec.subtitle = sub;
            }

            recordings << rec;
        }

        xmlXPathFreeObject(linkObj);
    }

    xmlXPathFreeObject(sectionObj);
    xmlFreeDoc(doc);
    return recordings;
}

// ---------------------------------------------------------------------------
// parseSendetipps
// ---------------------------------------------------------------------------
QList<Sendetipp> HtmlParser::parseSendetipps(const QString &html)
{
    QList<Sendetipp> tipps;

    xmlDocPtr doc = parseHtml(html);
    if (!doc) return tipps;

    // Each sendetipp is a <div class="item ..." itemprop="blogPost">
    xmlXPathObjectPtr itemObj = xpathEval(doc, nullptr,
        "//div[@itemprop='blogPost']");
    if (!itemObj) { xmlFreeDoc(doc); return tipps; }

    int itemCount = xpathNodeCount(itemObj);

    for (int i = 0; i < itemCount; ++i) {
        xmlNodePtr itemNode = xpathNode(itemObj, i);
        Sendetipp t;

        // Title
        xmlXPathObjectPtr titleObj = xpathEval(doc, itemNode,
            ".//h2[@itemprop='name']");
        if (titleObj && xpathNodeCount(titleObj) > 0)
            t.title = nodeText(xpathNode(titleObj, 0));
        if (titleObj) xmlXPathFreeObject(titleObj);

        // Show name + slug
        xmlXPathObjectPtr showObj = xpathEval(doc, itemNode,
            ".//a[contains(@href,'/sendungen/')]");
        if (showObj && xpathNodeCount(showObj) > 0) {
            xmlNodePtr showNode = xpathNode(showObj, 0);
            t.showName = nodeText(showNode);
            QString href = nodeAttr(showNode, "href");
            int lastSlash = href.lastIndexOf(QLatin1Char('/'));
            t.showSlug = (lastSlash >= 0) ? href.mid(lastSlash + 1) : href;
        }
        if (showObj) xmlXPathFreeObject(showObj);

        // Date/time from first <strong>
        xmlXPathObjectPtr strongObj = xpathEval(doc, itemNode,
            ".//strong[1]");
        if (strongObj && xpathNodeCount(strongObj) > 0)
            t.dateTime = nodeText(xpathNode(strongObj, 0));
        if (strongObj) xmlXPathFreeObject(strongObj);

        // Description: <p> that doesn't contain <strong>, <a>, or <img>
        xmlXPathObjectPtr descObj = xpathEval(doc, itemNode,
            ".//p[not(.//strong) and not(.//a) and not(.//img)]");
        if (descObj && xpathNodeCount(descObj) > 0)
            t.description = nodeText(xpathNode(descObj, 0));
        if (descObj) xmlXPathFreeObject(descObj);

        // Image
        xmlXPathObjectPtr imgObj = xpathEval(doc, itemNode,
            ".//img/@src");
        if (imgObj && xpathNodeCount(imgObj) > 0) {
            xmlNodePtr srcNode = xpathNode(imgObj, 0);
            // For attribute nodes, content is the attribute value
            if (srcNode->type == XML_ATTRIBUTE_NODE) {
                t.imageUrl = QString::fromUtf8(
                    reinterpret_cast<const char*>(srcNode->children->content));
            }
        }
        if (imgObj) xmlXPathFreeObject(imgObj);

        tipps << t;
    }

    xmlXPathFreeObject(itemObj);
    xmlFreeDoc(doc);
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