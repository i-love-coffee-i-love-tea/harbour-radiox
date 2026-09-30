#ifndef SITEFETCHER_H
#define SITEFETCHER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <functional>

class SiteFetcher : public QObject
{
    Q_OBJECT
public:
    explicit SiteFetcher(QObject *parent = nullptr, QNetworkAccessManager *nam = nullptr);

    bool loading() const;

    void fetchProgramWeek(int weekOffset = 0);
    void fetchRecordings();
    void fetchSendetipps();
    void fetchShowDetail(const QString &slug);
    void fetchPlayerPage(const QUrl &url);

signals:
    void programWeekReceived(const QString &html);
    void recordingsReceived(const QString &html);
    void sendetippsReceived(const QString &html);
    void showDetailReceived(const QString &html);
    void playerPageReceived(const QString &audioUrl);
    void networkError(const QString &errorString);
    void loadingChanged();

private:
    void startRequest(const QUrl &url,
                      std::function<void(const QString &)> onSuccess,
                      std::function<void(const QString &)> onError);

    QNetworkAccessManager *m_nam;
    int m_pendingRequests = 0;
};

#endif // SITEFETCHER_H