#ifndef __HTTP_SESSION_MANAGER_HPP__
#define __HTTP_SESSION_MANAGER_HPP__

#include <map>
#include <tuple>
#include <queue>
#include <memory>
#include <atomic>
#include <optional>
#include <fstream>

#include <QUrl>
#include <QNetworkReply>
#include <QHttpMultiPart>
#include <QNetworkAccessManager>
#include "NetworkConfigDialog.hpp"

#include "DownloadProgressWidget.hpp"

using HandleSessionCallBack = std::function<void(std::shared_ptr<QNetworkReply>)>;
using SessionEntry = std::tuple<std::shared_ptr<QNetworkReply>, HandleSessionCallBack>;

class HttpSessionManager: public QObject
{
    Q_OBJECT
public:
    HttpSessionManager(NetworkConfigDialog* networkConfigDialog, QObject *parent = nullptr);

    void postReqWithMultiPart(QHttpMultiPart* multiPart, QHttpPart* filePart, QJsonObject obj, HandleSessionCallBack callback);
    bool addSession(const uint32_t, QNetworkReply*, HandleSessionCallBack);
    void handleSessionWhenFinished(const uint32_t sessionId);

    QUrl getRqCacheDirUrl() const;
    QUrl getDownloadUrl() const;
    QUrl getUploadUrl() const;

    void pushEvent(QJsonObject obj);

signals:
    void canNotCreateAndOpenFile(std::string fileName);
    void downloadComplete(const SessionId, std::string);
    void downloadFailed(const SessionId, std::string fileName , std::string failureReason = "");

    void startDownload(const SessionId, std::string taskName);
    void bytesReaded(const SessionId, std::size_t size);

private:
    void postReqWithJsonType(QJsonObject obj);
    void handleNextRequest();

    std::ofstream createAndOpenFile(const std::string& fileName);

    std::optional<SessionEntry> getSessionEntry(const uint32_t sessionId);

    NetworkConfigDialog* networkConfigDialog_;
    std::shared_ptr<QNetworkAccessManager> networkdManager_;
    std::map<uint32_t, SessionEntry> sessionCache_;

    std::atomic<bool> requestOngoing_;
    std::queue<QJsonObject> requestQueue_;
};

#endif//__HTTP_SESSION_MANAGER_HPP__
