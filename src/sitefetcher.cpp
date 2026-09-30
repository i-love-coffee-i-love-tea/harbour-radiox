#include "sitefetcher.h"
#include "radioxsite.h"
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegularExpression>
#include <functional>

SiteFetcher::SiteFetcher(QObject *parent, QNetworkAccessManager *nam)
    : QObject(parent)
    , m_nam(nam ? nam : new QNetworkAccessManager(this))
{
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
        if (reply->error() != QNetworkReply::NoError) {
            onError(reply->errorString());
            return;
        }
        onSuccess(QString::fromUtf8(reply->readAll()));
    });
}

void SiteFetcher::fetchProgramWeek(int weekOffset)
{
    QString path = RadioXSite::kPathProgramWeek;
    if (weekOffset != 0)
        path += QStringLiteral("/%1").arg(weekOffset);
    QUrl url(RadioXSite::kBaseUrl + path);
    getAndEmit(m_nam, url, this,
        [this](const QString &html) { emit programWeekReceived(html); },
        [this](const QString &err) { emit networkError(err); });
}

void SiteFetcher::fetchRecordings()
{
    QUrl url(RadioXSite::kBaseUrl + RadioXSite::kPathRecordings);
    getAndEmit(m_nam, url, this,
        [this](const QString &html) { emit recordingsReceived(html); },
        [this](const QString &err) { emit networkError(err); });
}

void SiteFetcher::fetchSendetipps()
{
    QUrl url(RadioXSite::kBaseUrl + RadioXSite::kPathSendetipps);
    getAndEmit(m_nam, url, this,
        [this](const QString &html) { emit sendetippsReceived(html); },
        [this](const QString &err) { emit networkError(err); });
}

void SiteFetcher::fetchShowDetail(const QString &slug)
{
    QUrl url(RadioXSite::kBaseUrl + RadioXSite::kPathShowDetail.arg(slug));
    getAndEmit(m_nam, url, this,
        [this](const QString &html) { emit showDetailReceived(html); },
        [this](const QString &err) { emit networkError(err); });
}

void SiteFetcher::fetchPlayerPage(const QUrl &url)
{
    getAndEmit(m_nam, url, this,
        [this](const QString &html) {
            // Try <audio src="..."> or <source src="...">
            static QRegularExpression srcRe(
                RadioXSite::kReAudioSrc,
                QRegularExpression::CaseInsensitiveOption);
            QRegularExpressionMatch m = srcRe.match(html);
            if (m.hasMatch()) {
                emit playerPageReceived(m.captured(1));
                return;
            }
            // Try any URL that looks like an audio stream
            static QRegularExpression audioRe(
                RadioXSite::kReAudioFile,
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