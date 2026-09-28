#ifndef NETWORKFETCHER_H
#define NETWORKFETCHER_H

#include <QObject>
#include <QNetworkAccessManager>

class NetworkFetcher : public QObject
{
    Q_OBJECT
public:
    explicit NetworkFetcher(QObject *parent = nullptr, QNetworkAccessManager *nam = nullptr);

    void fetchProgramWeek(int weekOffset = 0);
    void fetchRecordings();
    void fetchSendetipps();
    void fetchShowDetail(const QString &slug);

    void setBaseUrl(const QString &url);
    QString baseUrl() const;

signals:
    void programWeekReceived(const QString &html);
    void recordingsReceived(const QString &html);
    void sendetippsReceived(const QString &html);
    void showDetailReceived(const QString &html);
    void networkError(const QString &errorString);

private:
    QNetworkAccessManager *m_nam;
    bool m_ownNam;
    QString m_baseUrl;
};

#endif // NETWORKFETCHER_H