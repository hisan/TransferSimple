#ifndef _FILE_TRANSFER_HANDLER_H_
#define _FILE_TRANSFER_HANDLER_H_

#include <map>
#include <string>

#include <QObject>
#include <QThread>

#include "NetworkConfigDialog.hpp"
#include "HttpSessionManager.hpp"
#include "DownloadProgressWidget.hpp"

class FileTransferHandler : public QObject
{
    Q_OBJECT
public:
    FileTransferHandler(DownloadProgressWidget* processWidget,
        NetworkConfigDialog* networkConfigDialog, QObject *parent = nullptr);

    ~FileTransferHandler();

    void uploadFile(const std::string& localPath, const std::string &filename);
    void downloadFile(const std::string& localPath, const std::string &fileName);
    void switchSubDir(const std::string& subDirPathName);

signals:
    void uploadComplete(std::string);
    void uploadFailed(std::string fileName, std::string failureReason);

    void downloadComplete(std::string);
    void downloadFailed(std::string fileName , std::string failureReason);

public slots:
    void requestSuccess(const SessionId, const std::string taskName);
    void requestFailed(const SessionId, const std::string taskName, const std::string errorInfo);

private:
    uint32_t generateSessionId();
    DownloadProgressWidget* processWidget_;
    NetworkConfigDialog* networkConfigDialog_;
    HttpSessionManager* sessionManager_;
    QThread* asyncaTransThread_;
};

#endif // _FILE_TRANSFER_HANDLER_H_
