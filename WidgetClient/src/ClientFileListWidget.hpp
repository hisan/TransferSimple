#ifndef __CLIENT_FILE_LIST_WIDGET_HPP__
#define __CLIENT_FILE_LIST_WIDGET_HPP__

#include <QTimer>
#include <QLineEdit>
#include "FileListWidget.hpp"
#include "DirectoryHandler.hpp"
#include "FileTransferHandler.hpp"

class ClientFileListWidget: public FileListWidget
{
public:
    ClientFileListWidget(
        FileTransferHandler* fileTransferHandler_,
        QLineEdit* ClientFileListWidget, QWidget* parent = nullptr);

public slots:
    void handleWhenItemDoubleClicked(QListWidgetItem* clickedItem) override;
    void refreshLocalFileItem(std::string fakeInfo = "");

    void hintWhenUploadSuccess(std::string fileName);
    void hintWhenUploadFailed(std::string fileName, std::string failureReason);

private:
    void reloadFileItems(const std::vector<std::tuple<std::string, std::string>>& fileInfos);

    FileTransferHandler* fileTransferHandler_;
    QLineEdit* clientDirInputWidget_;

    DirectoryHandler* directoryHandler_;
    QTimer inputStopTimer_;
};

#endif//__CLIENT_FILE_LIST_WIDGET_HPP__
