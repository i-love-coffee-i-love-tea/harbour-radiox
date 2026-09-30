#include "htmlparser.h"
#include "radioxsite.h"
#include <libxml/HTMLparser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include <QRegularExpression>
#include <QTextDocument>

// ---------------------------------------------------------------------------
// XPath selectors — update these when radiox.de HTML structure changes
// ---------------------------------------------------------------------------
namespace {
// Program week (schedule table)
const char* kXPath_WeekLabel     = "//div[@class='program-week-header']/p";
const char* kXPath_DayHeaders    = "//th[@class='day']";
const char* kXPath_ScheduleRows  = "//table[@id='weektable']//tbody/tr[@class='tbodytr']";
const char* kXPath_HourCell      = "td[contains(@class,'time_show_week')]";
const char* kXPath_DayCell       = "td[contains(@class,'hour_')]";
const char* kXPath_ShowDiv       = "div[@class='show_week_hour']";
const char* kXPath_FirstLink     = "a[1]";
const char* kXPath_SendetippLink = ".//a[contains(@href,'sendetipps')]";

// Recordings (plus7 archive)
const char* kXPath_DaySection    = "//p[contains(@class,'plus7-day')]";
const char* kXPath_RecordingLink = ".//a[contains(@onclick,'plus7_show_recording')]";

// Sendetipps (broadcast tips)
const char* kXPath_BlogPost      = "//div[@itemprop='blogPost']";
const char* kXPath_TippTitle     = ".//h2[@itemprop='name']";
const char* kXPath_TippShowLink  = ".//a[contains(@href,'/sendungen/')]";
const char* kXPath_TippDateTime  = ".//strong[1]";
const char* kXPath_TippDesc      = ".//p[not(.//strong) and not(.//a) and not(.//img)]";
const char* kXPath_TippImage     = ".//img/@src";

// Show detail page
const char* kXPath_ShowTitle     = "//h1[@itemprop='name']";
const char* kXPath_ArticleBody   = "//div[@itemprop='articleBody']";
const char* kXPath_ArticleImage  = "//div[@itemprop='articleBody']//img/@src";

// Text markers in schedule cells
const char* kMarker_Dito         = "_dito";
const char* kMarker_Repeat       = "(Wdh.)";
const char* kMarker_Sendetipp    = "Sendetipp";

// Regex patterns
const char* kRe_Time             = "(\\d{2}:\\d{2})";
const char* kRe_Date             = "(\\d{2}\\.\\d{2}\\.\\d{4})";
const char* kRe_RecordingId      = "plus7_show_recording\\((\\d+)\\)";
const char* kRe_SendetippBracket = "\\[.*?Sendetipp.*?\\]";
} // namespace

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
    static const QRegularExpression re{QLatin1String(kRe_Time)};
    for (xmlNodePtr cur = node->prev; cur; cur = cur->prev) {
        if (cur->type == XML_TEXT_NODE) {
            QString text = QString::fromUtf8(reinterpret_cast<const char*>(cur->content));
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
    static const QRegularExpression sendetippRe{QLatin1String(kRe_SendetippBracket)};

    QList<ProgramDay> days;
    weekLabel.clear();

    xmlDocPtr doc = parseHtml(html);
    if (!doc) return days;

    // 1. Week label
    xmlXPathObjectPtr weekObj = xpathEval(doc, nullptr, kXPath_WeekLabel);
    if (weekObj) {
        xmlNodePtr weekNode = xpathNode(weekObj, 0);
        if (weekNode)
            weekLabel = nodeText(weekNode);
        xmlXPathFreeObject(weekObj);
    }

    // 2. Day labels
    xmlXPathObjectPtr dayObj = xpathEval(doc, nullptr, kXPath_DayHeaders);
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
    xmlXPathObjectPtr rowObj = xpathEval(doc, nullptr, kXPath_ScheduleRows);
    if (!rowObj) { xmlFreeDoc(doc); return days; }

    int rowCount = xpathNodeCount(rowObj);

    for (int r = 0; r < rowCount; ++r) {
        xmlNodePtr rowNode = xpathNode(rowObj, r);

        // Hour from first time cell
        xmlXPathObjectPtr hourObj = xpathEval(doc, rowNode, kXPath_HourCell);
        if (!hourObj) continue;

        int hourCount = xpathNodeCount(hourObj);
        if (hourCount < 1) { xmlXPathFreeObject(hourObj); continue; }

        QString hourStr = nodeText(xpathNode(hourObj, 0));
        bool ok;
        int hour = hourStr.toInt(&ok);
        xmlXPathFreeObject(hourObj);
        if (!ok) continue;

        // Day cells (cells with class containing 'hour_')
        xmlXPathObjectPtr cellObj = xpathEval(doc, rowNode, kXPath_DayCell);
        if (!cellObj) continue;

        int cellCount = xpathNodeCount(cellObj);

        for (int d = 0; d < 7 && d < cellCount; ++d) {
            xmlNodePtr cellNode = xpathNode(cellObj, d);
            ScheduleSlot slot;
            slot.hour = hour;

            QString cellClass = nodeAttr(cellNode, "class");

            // Dito check
            if (cellClass.contains(QLatin1String(kMarker_Dito))) {
                slot.isDito = true;
                days[d].entries << slot;
                continue;
            }

            // Get show_week_hour div content
            xmlXPathObjectPtr divObj = xpathEval(doc, cellNode, kXPath_ShowDiv);
            if (!divObj || xpathNodeCount(divObj) == 0) {
                if (divObj) xmlXPathFreeObject(divObj);
                days[d].entries << slot;
                continue;
            }
            xmlNodePtr divNode = xpathNode(divObj, 0);

            // Full text of the div (for repeat/sendetipp checks)
            QString fullText = nodeText(divNode);

            // Show name from <a>
            xmlXPathObjectPtr aObj = xpathEval(doc, divNode, kXPath_FirstLink);
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
                slot.showName.remove(QLatin1String(kMarker_Repeat));
                slot.showName.remove(sendetippRe);
                slot.showName = slot.showName.trimmed();
            }

            // Repeat check
            slot.isRepeat = fullText.contains(QLatin1String(kMarker_Repeat));

            // Subtitle: text after <br/> in the div
            xmlNodePtr brNode = findNextElement(divNode->children, "br");
            if (brNode) {
                QString afterBr = textAfterElement(brNode);
                // Strip (Wdh.) and [Sendetipp...] markup
                afterBr.remove(QLatin1String(kMarker_Repeat));
                afterBr.remove(sendetippRe);
                slot.subtitle = afterBr.trimmed();
            }

            // Sendetipp link
            if (fullText.contains(QLatin1String(kMarker_Sendetipp))) {
                xmlXPathObjectPtr stObj = xpathEval(doc, divNode, kXPath_SendetippLink);
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
    xmlXPathObjectPtr sectionObj = xpathEval(doc, nullptr, kXPath_DaySection);
    if (!sectionObj) { xmlFreeDoc(doc); return recordings; }

    int sectionCount = xpathNodeCount(sectionObj);

    for (int s = 0; s < sectionCount; ++s) {
        xmlNodePtr sectionNode = xpathNode(sectionObj, s);

        // Extract date from text content: "Montag, 28.09.2026"
        static const QRegularExpression dateRe{QLatin1String(kRe_Date)};
        QString sectionText = nodeText(sectionNode);
        QRegularExpressionMatch dateMatch = dateRe.match(sectionText);
        QDate sectionDate;
        if (dateMatch.hasMatch())
            sectionDate = QDate::fromString(dateMatch.captured(1), QStringLiteral("dd.MM.yyyy"));

        // Find all recording links within this section
        xmlXPathObjectPtr linkObj = xpathEval(doc, sectionNode, kXPath_RecordingLink);
        if (!linkObj) continue;

        int linkCount = xpathNodeCount(linkObj);

        for (int i = 0; i < linkCount; ++i) {
            xmlNodePtr aNode = xpathNode(linkObj, i);
            Recording rec;
            rec.date = sectionDate;

            // Show name
            rec.showName = nodeText(aNode);

            // Recording ID from onclick
            static const QRegularExpression idRe{QLatin1String(kRe_RecordingId)};
            QString onclick = nodeAttr(aNode, "onclick");
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
    xmlXPathObjectPtr itemObj = xpathEval(doc, nullptr, kXPath_BlogPost);
    if (!itemObj) { xmlFreeDoc(doc); return tipps; }

    int itemCount = xpathNodeCount(itemObj);

    for (int i = 0; i < itemCount; ++i) {
        xmlNodePtr itemNode = xpathNode(itemObj, i);
        Sendetipp t;

        // Title
        xmlXPathObjectPtr titleObj = xpathEval(doc, itemNode, kXPath_TippTitle);
        if (titleObj && xpathNodeCount(titleObj) > 0)
            t.title = nodeText(xpathNode(titleObj, 0));
        if (titleObj) xmlXPathFreeObject(titleObj);

        // Show name + slug
        xmlXPathObjectPtr showObj = xpathEval(doc, itemNode, kXPath_TippShowLink);
        if (showObj && xpathNodeCount(showObj) > 0) {
            xmlNodePtr showNode = xpathNode(showObj, 0);
            t.showName = nodeText(showNode);
            QString href = nodeAttr(showNode, "href");
            int lastSlash = href.lastIndexOf(QLatin1Char('/'));
            t.showSlug = (lastSlash >= 0) ? href.mid(lastSlash + 1) : href;
        }
        if (showObj) xmlXPathFreeObject(showObj);

        // Date/time from first <strong>
        xmlXPathObjectPtr strongObj = xpathEval(doc, itemNode, kXPath_TippDateTime);
        if (strongObj && xpathNodeCount(strongObj) > 0)
            t.dateTime = nodeText(xpathNode(strongObj, 0));
        if (strongObj) xmlXPathFreeObject(strongObj);

        // Description: <p> that doesn't contain <strong>, <a>, or <img>
        xmlXPathObjectPtr descObj = xpathEval(doc, itemNode, kXPath_TippDesc);
        if (descObj && xpathNodeCount(descObj) > 0)
            t.description = nodeText(xpathNode(descObj, 0));
        if (descObj) xmlXPathFreeObject(descObj);

        // Image
        xmlXPathObjectPtr imgObj = xpathEval(doc, itemNode, kXPath_TippImage);
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
// parseShowDetail
// ---------------------------------------------------------------------------
static QString htmlToPlainText(const QString &html)
{
    QTextDocument doc;
    doc.setHtml(html);
    QString text = doc.toPlainText();
    text.replace(QRegularExpression(QStringLiteral("\n{3,}")), QStringLiteral("\n\n"));
    return text.trimmed();
}

QVariantMap HtmlParser::parseShowDetail(const QString &html)
{
    QVariantMap result;

    xmlDocPtr doc = parseHtml(html);
    if (!doc) return result;

    // Title
    xmlXPathObjectPtr titleObj = xpathEval(doc, nullptr, kXPath_ShowTitle);
    if (titleObj && xpathNodeCount(titleObj) > 0) {
        result[QStringLiteral("title")] = nodeText(xpathNode(titleObj, 0));
        xmlXPathFreeObject(titleObj);
    } else if (titleObj) {
        xmlXPathFreeObject(titleObj);
    }

    // Article body
    xmlXPathObjectPtr bodyObj = xpathEval(doc, nullptr, kXPath_ArticleBody);
    if (bodyObj && xpathNodeCount(bodyObj) > 0) {
        xmlNodePtr bodyNode = xpathNode(bodyObj, 0);
        // Get inner HTML by serializing child nodes
        xmlBufferPtr buf = xmlBufferCreate();
        for (xmlNodePtr child = bodyNode->children; child; child = child->next)
            xmlNodeDump(buf, doc, child, 0, 0);
        QString bodyHtml = QString::fromUtf8(
            reinterpret_cast<const char*>(xmlBufferContent(buf)));
        xmlBufferFree(buf);
        result[QStringLiteral("description")] = htmlToPlainText(bodyHtml);
        xmlXPathFreeObject(bodyObj);
    } else if (bodyObj) {
        xmlXPathFreeObject(bodyObj);
    }

    // Image
    xmlXPathObjectPtr imgObj = xpathEval(doc, nullptr, kXPath_ArticleImage);
    if (imgObj && xpathNodeCount(imgObj) > 0) {
        xmlNodePtr srcNode = xpathNode(imgObj, 0);
        if (srcNode->type == XML_ATTRIBUTE_NODE && srcNode->children) {
            QString src = QString::fromUtf8(
                reinterpret_cast<const char*>(srcNode->children->content));
            if (!src.startsWith(QStringLiteral("http")))
                src = RadioXSite::kBaseUrl() + src;
            result[QStringLiteral("imageUrl")] = src;
        }
        xmlXPathFreeObject(imgObj);
    } else if (imgObj) {
        xmlXPathFreeObject(imgObj);
    }

    xmlFreeDoc(doc);
    return result;
}