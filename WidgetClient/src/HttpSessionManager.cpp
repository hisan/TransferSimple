#include <filesystem>
#include <QStandardPaths>
#include <QFile>
#include "HttpSessionManager.hpp"

HttpSessionManager::HttpSessionManager(NetworkConfigDialog* networkConfigDialog, QObject *parent)
    : networkConfigDialog_(networkConfigDialog)
    , networkdManager_(std::make_shared<QNetworkAccessManager>())
    , requestOngoing_(false)
    , QObject(parent)
{
}

QUrl HttpSessionManager::getRqCacheDirUrl() const
{
    auto ip = networkConfigDialog_->getIp();
    auto port = networkConfigDialog_->getPort();
    QString url = QString("http://%1:%2/getCacheDir")
        .arg(QString(ip.c_str()))
        .arg(QString(port.c_str()));
    return QUrl(url);
}

QUrl HttpSessionManager::getDownloadUrl() const
{
    auto ip = networkConfigDialog_->getIp();
    auto port = networkConfigDialog_->getPort();
    auto secCode = networkConfigDialog_->getSecCode();
    QString downloadUrl = QString("http://%1:%2/download")
        .arg(QString(ip.c_str()))
        .arg(QString(port.c_str()));
    return QUrl(downloadUrl);
}

QUrl HttpSessionManager::getUploadUrl() const
{
    auto ip = networkConfigDialog_->getIp();
    auto port = networkConfigDialog_->getPort();
    auto secCode = networkConfigDialog_->getSecCode();
    QString uploadUrl = QString("http://%1:%2/upload")
        .arg(QString(ip.c_str()))
        .arg(QString(port.c_str()));

    return QUrl(uploadUrl);
}

bool HttpSessionManager::addSession(const uint32_t id, QNetworkReply* reply, HandleSessionCallBack callBack)
{
    if (sessionCache_.count(id) || !reply)
        return false;
    std::shared_ptr<QNetworkReply> replyPtr(reply);
    sessionCache_.insert({id, {replyPtr, callBack}});
    return true;
}

std::optional<SessionEntry> HttpSessionManager::getSessionEntry(const uint32_t sessionId)
{
    auto entryItr = sessionCache_.find(sessionId);
    if (entryItr == sessionCache_.end())
        return std::nullopt;
    return entryItr->second;
}

void HttpSessionManager::handleSessionWhenFinished(const uint32_t sessionId)
{
    auto optSessionEntry = getSessionEntry(sessionId);
    if (!optSessionEntry)
        return;
    auto& [reply, callBack] = optSessionEntry.value();
    if (callBack)
        callBack(reply);
    sessionCache_.erase(sessionId);
}

std::ofstream HttpSessionManager::createAndOpenFile(const std::string& fileName) 
{
    std::filesystem::path path = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation).toStdString();
    path /= fileName.c_str();
    std::ofstream ofs(path.string(), std::ofstream::binary | std::ofstream::out);
    return ofs;
}

void HttpSessionManager::handleNextRequest()
{
    if (!requestOngoing_.load() && !requestQueue_.empty())
    {
        auto obj = requestQueue_.front();
        requestQueue_.pop();
        requestOngoing_.store(true);
        postReqWithJsonType(obj);
    }
}

void HttpSessionManager::pushEvent(QJsonObject obj)
{
    static std::size_t taskCnt = 0;
    std::cout << "task[" << taskCnt++ << "] ongoing!\n";
    requestQueue_.push(obj);
    if (!requestOngoing_.load())
        handleNextRequest();
}

void HttpSessionManager::postReqWithJsonType(QJsonObject obj)
{
    auto url = this->getDownloadUrl();
    QNetworkRequest request{url};
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    auto sessionId = std::atoi(obj["sessionId"].toString().toStdString().c_str());
    auto taskName = obj["fileName"].toString().toStdString();

    auto file = new std::ofstream(std::move(createAndOpenFile(taskName)));
    if (not file->is_open())
    {
        std::string info = "can not create file for storing response data!";
        qWarning() << info.c_str();
        emit canNotCreateAndOpenFile(taskName);
        return;
    }

    emit startDownload(sessionId, taskName);

    auto reply = networkdManager_->post(request, QJsonDocument(obj).toJson());
    if (reply)
    {
        connect(reply, &QNetworkReply::readyRead, [taskName, sessionId, file, reply, this]()
        {
            std::uint64_t totalCnt = 0;
            while (reply->bytesAvailable())
            {
                QByteArray responData = reply->read(4096);
                std::string byteArray(responData.constData(), responData.size());
                *file << byteArray;
                totalCnt += byteArray.size();
            }

            emit bytesReaded(sessionId, totalCnt);
            // std::cout << "writed data: " << totalCnt << "\n";
        });

        connect(reply, &QNetworkReply::finished, [sessionId, taskName, file, reply, this]()
        {
            std::cout << "download file: " << taskName << " finished!\n";
            if (reply->error() == QNetworkReply::NoError)
                emit downloadComplete(sessionId, taskName);
            else
                emit downloadFailed(sessionId, taskName, reply->errorString().toStdString());

            file->close();
            delete file;
            requestOngoing_.store(false);
            handleNextRequest();
        });
    }
    else 
    {
        requestOngoing_.store(false);
        handleNextRequest();
        std::string info = "can not create request";
        qWarning() << info.c_str();
        emit downloadFailed(sessionId, taskName, info);
    }
}

void HttpSessionManager::postReqWithMultiPart(
    QHttpMultiPart* multiPart,
    QHttpPart* filePart,
    QJsonObject obj,
    HandleSessionCallBack callback)
{
    auto url = this->getUploadUrl();
    QNetworkRequest request(url);

    QHttpPart metadataPart;
    metadataPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"metadata\""));

    QJsonDocument doc(obj);
    metadataPart.setBody(doc.toJson(QJsonDocument::Compact));

    multiPart->append(metadataPart);
    multiPart->append(*filePart);

    auto sessionId = std::atoi(obj["sessionId"].toString().toStdString().c_str());
    auto reply = networkdManager_->post(request, multiPart);

    if (reply && addSession(sessionId, reply, callback))
    {
        multiPart->setParent(reply);// in order to delete multiPart when delete the QNetworkReply
        connect(reply, &QNetworkReply::finished, [sessionId, this]() {
            this->handleSessionWhenFinished(sessionId);
        });
    }
    else
        delete multiPart;
}