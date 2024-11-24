#include <QMessageBox>
#include "FileListItem.hpp"
#include "ServerFileListWidget.hpp"

ServerFileListWidget::ServerFileListWidget(
        FileTransferHandler* fileTransferHandler,
        QLineEdit* clientDirInputWidget,
        QWidget* parent
    )
    : clientDirInputWidget_(clientDirInputWidget)
    , fileTransferHandler_(fileTransferHandler)
    , FileListWidget(parent)
{
    connect(fileTransferHandler_, &FileTransferHandler::downloadComplete,
        this, &ServerFileListWidget::hintWhenDownloadSuccess);

    connect(fileTransferHandler_, &FileTransferHandler::downloadFailed,
        this, &ServerFileListWidget::hintWhenDownloadFailed);
}

void ServerFileListWidget::hintWhenDownloadSuccess(std::string fileName)
{
    std::string info = "Download successful:" + fileName;
    QMessageBox::about(this, "DOWNLOAD INFO", info.c_str());
}

void ServerFileListWidget::hintWhenDownloadFailed(std::string fileName, std::string failureReason)
{
    std::string warningStr = "Download file: " + fileName + " failed!\nReason: " + failureReason;
    QMessageBox::warning(this, "OWNLOAD INFO", warningStr.c_str());
}

void ServerFileListWidget::handleWhenItemDoubleClicked(QListWidgetItem* clickedItem)
{
    auto fileListItem = dynamic_cast<FileListItem*>(clickedItem);
    auto fileName = fileListItem->getFileName();

    if (fileListItem->isDir())
    {
        // #ToDo: let server switch dir
        fileTransferHandler_->switchSubDir(fileName);
    }
    else
    {
        fileTransferHandler_->downloadFile(clientDirInputWidget_->text().toStdString(), fileName);
    }
}