#include "radioxcore.h"
#include <QUrl>

RadioXCore::RadioXCore(QObject *parent)
    : QObject(parent)
    , m_programModel(new ProgramModel(this))
    , m_recordingsModel(new RecordingsModel(this))
    , m_sendetippsModel(new SendetippsModel(this))
    , m_fetcher(new NetworkFetcher(this))
{
    connect(m_fetcher, &NetworkFetcher::programWeekReceived,
            m_programModel, &ProgramModel::loadFromHtml);
    connect(m_fetcher, &NetworkFetcher::recordingsReceived,
            m_recordingsModel, &RecordingsModel::loadFromHtml);
    connect(m_fetcher, &NetworkFetcher::sendetippsReceived,
            m_sendetippsModel, &SendetippsModel::loadFromHtml);
    connect(m_fetcher, &NetworkFetcher::networkError, this, [this](const QString &err) {
        m_errorMessage = err;
        emit errorMessageChanged();
        emit error(err);
    });

    // Clear error when data arrives
    connect(m_fetcher, &NetworkFetcher::programWeekReceived, this, [this]() {
        m_lastInfo = QStringLiteral("Schedule loaded");
        emit lastInfoChanged();
        if (!m_errorMessage.isEmpty()) {
            m_errorMessage.clear();
            emit errorMessageChanged();
        }
    });
    connect(m_fetcher, &NetworkFetcher::recordingsReceived, this, [this]() {
        m_lastInfo = QStringLiteral("Recordings loaded");
        emit lastInfoChanged();
    });

    // Set loading=false when any data arrives or on error
    auto finishLoading = [this]() {
        if (m_loading) {
            m_loading = false;
            emit loadingChanged();
        }
    };
    connect(m_fetcher, &NetworkFetcher::programWeekReceived, this, finishLoading);
    connect(m_fetcher, &NetworkFetcher::recordingsReceived, this, finishLoading);
    connect(m_fetcher, &NetworkFetcher::sendetippsReceived, this, finishLoading);
    connect(m_fetcher, &NetworkFetcher::networkError, this, finishLoading);
}

ProgramModel* RadioXCore::programModel() const { return m_programModel; }
RecordingsModel* RadioXCore::recordingsModel() const { return m_recordingsModel; }
SendetippsModel* RadioXCore::sendetippsModel() const { return m_sendetippsModel; }
bool RadioXCore::loading() const { return m_loading; }

QString RadioXCore::livestreamUrl() const
{
    return QStringLiteral("https://stream.radiox.de/live.ogg");
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
    m_loading = true;
    emit loadingChanged();
    m_lastInfo = QStringLiteral("Refreshing...");
    emit lastInfoChanged();
    m_fetcher->fetchProgramWeek(m_programModel->weekOffset());
    m_fetcher->fetchRecordings();
    m_fetcher->fetchSendetipps();
}

void RadioXCore::playRecording(int id)
{
    m_playbackUrl = QStringLiteral("https://www.radiox.de/plus7/ajax/player/%1").arg(id);
    emit playbackUrlChanged(m_playbackUrl);
    emit playbackStarted();
}

void RadioXCore::openLivestream()
{
    m_playbackUrl = livestreamUrl();
    emit playbackUrlChanged(m_playbackUrl);
    emit playbackStarted();
}

QString RadioXCore::playbackUrl() const
{
    return m_playbackUrl;
}

void RadioXCore::stopPlayback()
{
    m_playbackUrl.clear();
    emit playbackStopped();
}