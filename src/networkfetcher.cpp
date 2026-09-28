#include "networkfetcher.h"
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QFile>
#include <QTextStream>
#include <functional>

NetworkFetcher::NetworkFetcher(QObject *parent, QNetworkAccessManager *nam)
    : QObject(parent)
    , m_nam(nam ? nam : new QNetworkAccessManager(this))
    , m_ownNam(!nam)
    , m_baseUrl(QStringLiteral("https://www.radiox.de"))
{
}

void NetworkFetcher::setBaseUrl(const QString &url)
{
    m_baseUrl = url;
}

QString NetworkFetcher::baseUrl() const
{
    return m_baseUrl;
}

static void getAndEmit(QNetworkAccessManager *nam, const QUrl &url,
                        QObject *ctx, std::function<void(const QString &)> onSuccess,
                        std::function<void(const QString &)> onError)
{
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::UserAgentHeader, "harbour-radiox/0.1");
    QNetworkReply *reply = nam->get(req);
    QObject::connect(reply, &QNetworkReply::finished, ctx, [=]() {
        reply->deleteLater();
        // Write debug to file for device debugging
        QFile logFile("/tmp/harbour-radiox.log");
        logFile.open(QIODevice::Append | QIODevice::Text);
        QTextStream ts(&logFile);
        ts << "URL: " << url.toString() << "\n";
        ts << "Error: " << reply->error() << " " << reply->errorString() << "\n";
        if (reply->error() != QNetworkReply::NoError) {
            ts << "FAILED\n---\n";
            logFile.close();
            onError(reply->errorString());
            return;
        }
        QString html = QString::fromUtf8(reply->readAll());
        ts << "Bytes: " << html.size() << "\n";
        ts << "First 200: " << html.left(200) << "\n---\n";
        logFile.close();
        onSuccess(html);
    });
}

void NetworkFetcher::fetchProgramWeek(int weekOffset)
{
    QString path = QStringLiteral("/plus7/ajax/program_week");
    if (weekOffset != 0)
        path += QStringLiteral("/%1").arg(weekOffset);
    QUrl url(m_baseUrl + path);
    getAndEmit(m_nam, url, this,
        [this](const QString &html) { emit programWeekReceived(html); },
        [this](const QString &err) { emit networkError(err); });
}

void NetworkFetcher::fetchRecordings()
{
    QUrl url(m_baseUrl + QStringLiteral("/plus7/ajax/plus7_content_all"));
    getAndEmit(m_nam, url, this,
        [this](const QString &html) { emit recordingsReceived(html); },
        [this](const QString &err) { emit networkError(err); });
}

void NetworkFetcher::fetchSendetipps()
{
    QUrl url(m_baseUrl + QStringLiteral("/programm/sendetipps"));
    getAndEmit(m_nam, url, this,
        [this](const QString &html) { emit sendetippsReceived(html); },
        [this](const QString &err) { emit networkError(err); });
}

void NetworkFetcher::fetchShowDetail(const QString &slug)
{
    QUrl url(m_baseUrl + QStringLiteral("/sendungen/%1").arg(slug));
    getAndEmit(m_nam, url, this,
        [this](const QString &html) { emit showDetailReceived(html); },
        [this](const QString &err) { emit networkError(err); });
}