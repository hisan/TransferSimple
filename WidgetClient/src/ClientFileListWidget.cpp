#include "ClientFileListWidget.hpp"
#include "FileListItem.hpp"

ClientFileListWidget::ClientFileListWidget(
    FileTransferHandler* fileTransferHandler,
    QLineEdit* clientDirInputWidget,
    QWidget* parent)
    : fileTransferHandler_(fileTransferHandler)
    , clientDirInputWidget_(clientDirInputWidget)
    , directoryHandler_(new DirectoryHandler())
    , FileListWidget(parent)
{
    connect(clientDirInputWidget_, &QLineEdit::textEdited, this, [this](){
        this->inputStopTimer_.start(300);
    });
    connect(&inputStopTimer_, &QTimer::timeout, this, [this](){
        this->refreshLocalFileItem();
    });

    connect(fileTransferHandler_, &FileTransferHandler::uploadComplete,
        this, &ClientFileListWidget::hintWhenUploadSuccess);

    connect(fileTransferHandler_, &FileTransferHandler::uploadFailed,
        this, &ClientFileListWidget::hintWhenUploadFailed);

    this->refreshLocalFileItem();
}

void ClientFileListWidget::hintWhenUploadSuccess(std::string fileName)
{
    std::string info = "Upload Successful:" + fileName;
    QMessageBox::about(this, "UPLOAD INFO", info.c_str());
}

void ClientFileListWidget::hintWhenUploadFailed(std::string fileName, std::string failureReason)
{
    std::string info = "Upload: " + fileName + " failed!\nReason: " + failureReason;
    QMessageBox::warning(this, "UPLOAD INFO", info.c_str());
}

void ClientFileListWidget::handleWhenItemDoubleClicked(QListWidgetItem* clickedItem)
{
    auto curDir = clientDirInputWidget_->text().toStdString();
    if (*curDir.rbegin() == '/')
        curDir = curDir.substr(0, curDir.size() - 1);

    auto fileListItem = dynamic_cast<FileListItem*>(clickedItem);
    if (fileListItem->isDir())
    {
        clientDirInputWidget_->setText(std::string(curDir + "/" + fileListItem->getFileName()).c_str());
        refreshLocalFileItem();
    }
    else
    {
        fileTransferHandler_->uploadFile(curDir, fileListItem->getFileName());
    }
}

void ClientFileListWidget::refreshLocalFileItem(std::string fakeInfo)
{
    const auto tmpPath = clientDirInputWidget_->text().toStdString();
    if (directoryHandler_->validateAndLoadDirectory(tmpPath))
    {
        auto localFiles = directoryHandler_->loadDirectory(tmpPath);
        reloadFileItems(localFiles);
    }
    inputStopTimer_.stop();
}

void ClientFileListWidget::reloadFileItems(const std::vector<std::tuple<std::string, std::string>>& fileInfos)
{
    this->clear();
    for (const auto& [fileName, fileType] : fileInfos)
    {
        addItem(fileName, fileType);
    }
}
