#ifndef RADIOXCORE_H
#define RADIOXCORE_H

#include <QObject>
#include "programmodel.h"
#include "recordingsmodel.h"
#include "sendetippsmodel.h"
#include "networkfetcher.h"

class RadioXCore : public QObject
{
    Q_OBJECT
    Q_PROPERTY(ProgramModel* programModel READ programModel CONSTANT)
    Q_PROPERTY(RecordingsModel* recordingsModel READ recordingsModel CONSTANT)
    Q_PROPERTY(SendetippsModel* sendetippsModel READ sendetippsModel CONSTANT)
    Q_PROPERTY(bool loading READ loading NOTIFY loadingChanged)
    Q_PROPERTY(QString livestreamUrl READ livestreamUrl CONSTANT)
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY errorMessageChanged)
    Q_PROPERTY(QString lastInfo READ lastInfo NOTIFY lastInfoChanged)
    Q_PROPERTY(QString playbackUrl READ playbackUrl NOTIFY playbackUrlChanged)
    Q_PROPERTY(QString playbackTitle READ playbackTitle NOTIFY playbackTitleChanged)
    Q_PROPERTY(bool livestreamPlaying READ livestreamPlaying NOTIFY playbackUrlChanged)
    Q_PROPERTY(QVariantMap showDetail READ showDetail NOTIFY showDetailChanged)
    Q_PROPERTY(bool loadingShowDetail READ loadingShowDetail NOTIFY loadingShowDetailChanged)
public:
    explicit RadioXCore(QObject *parent = nullptr);

    ProgramModel* programModel() const;
    RecordingsModel* recordingsModel() const;
    SendetippsModel* sendetippsModel() const;
    bool loading() const;
    QString livestreamUrl() const;
    QString errorMessage() const;
    QString lastInfo() const;
    QString playbackUrl() const;
    QString playbackTitle() const;
    bool livestreamPlaying() const;
    QVariantMap showDetail() const;
    bool loadingShowDetail() const;

    Q_INVOKABLE void refresh();
    Q_INVOKABLE void fetchShowDetail(const QString &slug);
    Q_INVOKABLE void playRecording(int id, const QString &title);
    Q_INVOKABLE void openLivestream();
    Q_INVOKABLE void stopPlayback();

signals:
    void loadingChanged();
    void playbackUrlChanged(const QString &url);
    void playbackTitleChanged();
    void playbackStarted();
    void playbackStopped();
    void showDetailChanged();
    void loadingShowDetailChanged();
    void error(const QString &message);
    void errorMessageChanged();
    void lastInfoChanged();

private:
    ProgramModel *m_programModel;
    RecordingsModel *m_recordingsModel;
    SendetippsModel *m_sendetippsModel;
    NetworkFetcher *m_fetcher;
    bool m_loading = false;
    QString m_errorMessage;
    QString m_lastInfo;
    QString m_playbackUrl;
    QString m_playbackTitle;
    QVariantMap m_showDetail;
    bool m_loadingShowDetail = false;
};

#endif // RADIOXCORE_H