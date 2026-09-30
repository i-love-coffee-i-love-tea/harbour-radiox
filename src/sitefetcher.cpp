#include "sitefetcher.h"
#include "radioxsite.h"
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegularExpression>
#include <QTimer>
#include <functional>

static const int kRequestTimeoutMs = 10000;

SiteFetcher::SiteFetcher(QObject *parent, QNetworkAccessManager *nam)
    : QObject(parent)
    , m_nam(nam ? nam : new QNetworkAccessManager(this))
{
}

bool SiteFetcher::loading() const
{
    return m_pendingRequests > 0;
}

void SiteFetcher::startRequest(const QUrl &url,
                                std::function<void(const QString &)> onSuccess,
                                std::function<void(const QString &)> onError)
{
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::UserAgentHeader, "harbour-radiox/0.1");
    QNetworkReply *reply = m_nam->get(req);

    m_pendingRequests++;
    emit loadingChanged();

    // Abort on timeout — timer is parented to reply so it's cleaned up automatically
    QTimer *timer = new QTimer(reply);
    timer->setSingleShot(true);
    timer->setInterval(kRequestTimeoutMs);
    QObject::connect(timer, &QTimer::timeout, [reply]() { reply->abort(); });
    timer->start();

    QObject::connect(reply, &QNetworkReply::finished, this, [=]() {
        reply->deleteLater();
        m_pendingRequests--;
        emit loadingChanged();
        if (reply->error() != QNetworkReply::NoError) {
            onError(reply->errorString());
            return;
        }
        onSuccess(QString::fromUtf8(reply->readAll()));
    });
}

void SiteFetcher::fetchProgramWeek(int weekOffset)
{
    QString path = RadioXSite::kPathProgramWeek();
    if (weekOffset != 0)
        path += QStringLiteral("/%1").arg(weekOffset);
    QUrl url(RadioXSite::kBaseUrl() + path);
    startRequest(url,
        [this](const QString &html) { emit programWeekReceived(html); },
        [this](const QString &err) { emit networkError(err); });
}

void SiteFetcher::fetchRecordings()
{
    QUrl url(RadioXSite::kBaseUrl() + RadioXSite::kPathRecordings());
    startRequest(url,
        [this](const QString &html) { emit recordingsReceived(html); },
        [this](const QString &err) { emit networkError(err); });
}

void SiteFetcher::fetchSendetipps()
{
    QUrl url(RadioXSite::kBaseUrl() + RadioXSite::kPathSendetipps());
    startRequest(url,
        [this](const QString &html) { emit sendetippsReceived(html); },
        [this](const QString &err) { emit networkError(err); });
}

void SiteFetcher::fetchShowDetail(const QString &slug)
{
    QUrl url(RadioXSite::kBaseUrl() + RadioXSite::kPathShowDetail().arg(slug));
    startRequest(url,
        [this](const QString &html) { emit showDetailReceived(html); },
        [this](const QString &err) { emit networkError(err); });
}

void SiteFetcher::fetchPlayerPage(const QUrl &url)
{
    startRequest(url,
        [this](const QString &html) {
            // Try <audio src="..."> or <source src="...">
            static QRegularExpression srcRe(
                QLatin1String(RadioXSite::kReAudioSrc),
                QRegularExpression::CaseInsensitiveOption);
            QRegularExpressionMatch m = srcRe.match(html);
            if (m.hasMatch()) {
                emit playerPageReceived(m.captured(1));
                return;
            }
            // Try any URL that looks like an audio stream
            static QRegularExpression audioRe(
                QLatin1String(RadioXSite::kReAudioFile),
                QRegularExpression::CaseInsensitiveOption);
            m = audioRe.match(html);
            if (m.hasMatch()) {
                emit playerPageReceived(m.captured(1));
                return;
            }
            emit networkError(QStringLiteral("Could not find audio URL in player page"));
        },
        [this](const QString &err) { emit networkError(err); });
}