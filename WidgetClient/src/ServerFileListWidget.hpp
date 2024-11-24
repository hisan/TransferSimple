#ifndef __SERVER_FILE_LIST_WIDGET_HPP__
#define __SERVER_FILE_LIST_WIDGET_HPP__

#include <QLineEdit>
#include "FileListWidget.hpp"
#include "FileTransferHandler.hpp"

class ServerFileListWidget: public FileListWidget
{
public:
    ServerFileListWidget(
        FileTransferHandler* fileTransferHandler,
        QLineEdit* clientDirInputWidget,
        QWidget* parent = nullptr);

public slots:
    void handleWhenItemDoubleClicked(QListWidgetItem* clickedItem) override;
    void hintWhenDownloadFailed(std::string fileName, std::string failureReason);
    void hintWhenDownloadSuccess(std::string fileName);

private:
    QLineEdit* clientDirInputWidget_;
    FileTransferHandler* fileTransferHandler_;
};

#endif//__SERVER_FILE_LIST_WIDGET_HPP__
