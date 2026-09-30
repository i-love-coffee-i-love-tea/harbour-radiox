#include "radioxcore.h"
#include "radioxsite.h"
#include <QUrl>

RadioXCore::RadioXCore(QObject *parent)
    : QObject(parent)
    , m_programModel(new ProgramModel(this))
    , m_recordingsModel(new RecordingsModel(this))
    , m_sendetippsModel(new SendetippsModel(this))
    , m_fetcher(new SiteFetcher(this))
{
    connect(m_fetcher, &SiteFetcher::programWeekReceived,
            m_programModel, &ProgramModel::loadFromHtml);
    connect(m_fetcher, &SiteFetcher::recordingsReceived,
            m_recordingsModel, &RecordingsModel::loadFromHtml);
    connect(m_fetcher, &SiteFetcher::sendetippsReceived,
            m_sendetippsModel, &SendetippsModel::loadFromHtml);
    connect(m_fetcher, &SiteFetcher::playerPageReceived, this, [this](const QString &audioUrl) {
        m_playbackUrl = audioUrl;
        emit playbackUrlChanged(m_playbackUrl);
        emit playbackStarted();
    });
    connect(m_fetcher, &SiteFetcher::showDetailReceived, this, [this](const QString &html) {
        m_showDetail = HtmlParser::parseShowDetail(html);
        m_loadingShowDetail = false;
        emit showDetailChanged();
        emit loadingShowDetailChanged();
    });
    connect(m_fetcher, &SiteFetcher::networkError, this, [this](const QString &err) {
        m_errorMessage = err;
        emit errorMessageChanged();
    });

    // Clear error when data arrives
    connect(m_fetcher, &SiteFetcher::programWeekReceived, this, [this]() {
        m_lastInfo = QStringLiteral("Schedule loaded");
        emit lastInfoChanged();
        if (!m_errorMessage.isEmpty()) {
            m_errorMessage.clear();
            emit errorMessageChanged();
        }
    });
    connect(m_fetcher, &SiteFetcher::recordingsReceived, this, [this]() {
        m_lastInfo = QStringLiteral("Recordings loaded");
        emit lastInfoChanged();
    });
    connect(m_fetcher, &SiteFetcher::sendetippsReceived, this, [this]() {
        m_lastInfo = QStringLiteral("Sendetipps loaded");
        emit lastInfoChanged();
        if (!m_errorMessage.isEmpty()) {
            m_errorMessage.clear();
            emit errorMessageChanged();
        }
    });

    connect(m_fetcher, &SiteFetcher::loadingChanged,
            this, &RadioXCore::loadingChanged);
}

ProgramModel* RadioXCore::programModel() const { return m_programModel; }
RecordingsModel* RadioXCore::recordingsModel() const { return m_recordingsModel; }
SendetippsModel* RadioXCore::sendetippsModel() const { return m_sendetippsModel; }
bool RadioXCore::loading() const { return m_fetcher->loading(); }

QString RadioXCore::livestreamUrl() const
{
    return RadioXSite::kLivestreamUrl();
}

QString RadioXCore::baseUrl() const
{
    return RadioXSite::kBaseUrl();
}

QString RadioXCore::errorMessage() const
{
    return m_errorMessage;
}

QString RadioXCore::lastInfo() const
{
    return m_lastInfo;
}

void RadioXCore::refresh()
{
    m_lastInfo = QStringLiteral("Refreshing...");
    emit lastInfoChanged();
    m_fetcher->fetchProgramWeek(m_programModel->weekOffset());
    m_fetcher->fetchRecordings();
    m_fetcher->fetchSendetipps();
}

void RadioXCore::fetchShowDetail(const QString &slug)
{
    m_loadingShowDetail = true;
    emit loadingShowDetailChanged();
    m_fetcher->fetchShowDetail(slug);
}

void RadioXCore::playRecording(int id, const QString &title)
{
    m_playbackTitle = title;
    emit playbackTitleChanged();
    QUrl url(RadioXSite::kBaseUrl() + RadioXSite::kPathPlayerPage().arg(id));
    m_fetcher->fetchPlayerPage(url);
}

void RadioXCore::openLivestream()
{
    m_playbackTitle.clear();
    emit playbackTitleChanged();
    m_playbackUrl = livestreamUrl();
    emit playbackUrlChanged(m_playbackUrl);
    emit playbackStarted();
}

QString RadioXCore::playbackUrl() const
{
    return m_playbackUrl;
}

QString RadioXCore::playbackTitle() const
{
    return m_playbackTitle;
}

bool RadioXCore::livestreamPlaying() const
{
    return !m_playbackUrl.isEmpty() && m_playbackUrl == livestreamUrl();
}

QVariantMap RadioXCore::showDetail() const
{
    return m_showDetail;
}

bool RadioXCore::loadingShowDetail() const
{
    return m_loadingShowDetail;
}

void RadioXCore::stopPlayback()
{
    m_playbackUrl.clear();
    m_playbackTitle.clear();
    emit playbackTitleChanged();
    emit playbackUrlChanged(m_playbackUrl);
    emit playbackStopped();
}