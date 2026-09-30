#ifndef SITEFETCHER_H
#define SITEFETCHER_H

#include <QObject>
#include <QNetworkAccessManager>

class SiteFetcher : public QObject
{
    Q_OBJECT
public:
    explicit SiteFetcher(QObject *parent = nullptr, QNetworkAccessManager *nam = nullptr);

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

private:
    QNetworkAccessManager *m_nam;
};

#endif // SITEFETCHER_H