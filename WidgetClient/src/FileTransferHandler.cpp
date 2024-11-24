#include <filesystem>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QHttpMultiPart>
#include <QFile>
#include <QUrl>
#include <QStandardPaths>
#include <QFileDialog>
#include <QMessageBox>

#include "FileTransferHandler.hpp"

FileTransferHandler::FileTransferHandler(DownloadProgressWidget* processWidget, NetworkConfigDialog* networkConfigDialog, QObject *parent)
    : processWidget_(processWidget)
    , networkConfigDialog_(networkConfigDialog)
    , sessionManager_(new HttpSessionManager(networkConfigDialog_))
    , asyncaTransThread_(new QThread(this))
    , QObject(parent)
{
    connect(sessionManager_, &HttpSessionManager::downloadComplete, this, &FileTransferHandler::requestSuccess);
    connect(sessionManager_, &HttpSessionManager::downloadFailed, this, &FileTransferHandler::requestFailed);

    connect(sessionManager_, &HttpSessionManager::bytesReaded, processWidget_, &DownloadProgressWidget::updateDownloadProgress);

    connect(sessionManager_, &HttpSessionManager::downloadComplete, [this](const SessionId sessionId, std::string taskName)
    {
        this->processWidget_->removeDownloadTask(sessionId, taskName);
    });

    connect(sessionManager_, &HttpSessionManager::downloadFailed, processWidget_, &DownloadProgressWidget::removeDownloadTask);

    sessionManager_->moveToThread(asyncaTransThread_);
    asyncaTransThread_->start();
}

FileTransferHandler::~FileTransferHandler()
{
    asyncaTransThread_->quit();
    asyncaTransThread_->wait();
}

uint32_t FileTransferHandler::generateSessionId()
{
    static uint32_t id = 1;
    return id++;
}

void FileTransferHandler::uploadFile(const std::string& localPath, const std::string &filename)
{
    std::filesystem::path filePath = localPath;
    auto absFileName = filePath / filename;
    qDebug() << __FUNCTION__ << ", file: " << localPath.c_str() << "/" << filename.c_str() << "\n";

    if (not networkConfigDialog_->isOnline())
    {
        qDebug() << __FUNCTION__ << "connection lost, please reconnect!\n";
        emit uploadFailed(absFileName.string(), "Please connect server first!");
        return;
    }

    auto file = new QFile(absFileName);
    if (!file->open(QIODevice::ReadOnly)) {
        qWarning() << "Unable to open file";
        delete file;
        emit uploadFailed(absFileName.string(), "Unable to open file");
        return;
    }

    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    file->setParent(multiPart);//in order to delete QFile when delete multipart

    auto *filePart = new QHttpPart();
    filePart->setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"file\"; filename=\"" + QString(filename.c_str()) + "\""));
    filePart->setBodyDevice(file);

    QJsonObject json;
    json["sessionId"] = std::to_string(generateSessionId()).c_str();
    json["secCode"] = networkConfigDialog_->getSecCode().c_str();
    json["fileName"] = filename.c_str();

    auto responseCall = [this, filePath](std::shared_ptr<QNetworkReply> reply)
    {
        if (reply->error() == QNetworkReply::NoError)
        {
            qDebug() << "File uploaded successfully";
            emit uploadComplete(filePath);
        }
        else
        {
            qWarning() << "Upload failed: " << reply->errorString();
            emit uploadFailed(filePath, reply->errorString().toStdString());
        }
    };
    sessionManager_->postReqWithMultiPart(multiPart, filePart, json, responseCall);
}

void FileTransferHandler::downloadFile(const std::string& localPath, const std::string &fileName)
{
    QJsonObject json;
    auto sessionId = generateSessionId();
    json["sessionId"] = std::to_string(sessionId).c_str();
    json["secCode"] = networkConfigDialog_->getSecCode().c_str();
    json["fileName"] = fileName.c_str();

    auto filePath = std::filesystem::path(localPath) / fileName;
    std::cout << __FUNCTION__ << ", filePath:" << filePath << "\n";

    processWidget_->addDownloadTask(sessionId, TaskItem(fileName, 0, 0));
    sessionManager_->pushEvent(json);
}

void FileTransferHandler::switchSubDir(const std::string& subDirPathName)
{
    //switch dir
}

void FileTransferHandler::requestSuccess(const SessionId, std::string taskName)
{
    emit downloadComplete(taskName);
}

void FileTransferHandler::requestFailed(const SessionId, const std::string taskName, const std::string errorInfo)
{
    emit downloadFailed(taskName, errorInfo);
}
