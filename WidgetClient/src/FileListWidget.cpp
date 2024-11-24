#include "FileListItem.hpp"
#include "FileListWidget.hpp"

FileListWidget::FileListWidget(QWidget* parent)
    : QListWidget(parent)
{
    #ifdef Q_OS_ANDROID
        connect(this, &QListWidget::itemClicked, this, &FileListWidget::handleWhenItemDoubleClicked);
    #else
        connect(this, &QListWidget::itemDoubleClicked, this, &FileListWidget::handleWhenItemDoubleClicked);
    #endif
}

void FileListWidget::addItem(const std::string& fileName, const std::string& fileType)
{
    auto fileItem = new FileListItem(fileName, fileType, this);
    if (fileType == "file")
    {
        if (fileName.ends_with(".mp3")) {
            fileItem->setIcon(QIcon(std::string(":/images/mp3.png").c_str()));
        }
        else if (fileName.ends_with(".mp4")) {
            fileItem->setIcon(QIcon(std::string(":/images/mp4.png").c_str()));
        }
        else if (fileName.ends_with(".pdf")) {
            fileItem->setIcon(QIcon(std::string(":/images/pdf.png").c_str()));
        }
        else
            fileItem->setIcon(QIcon(std::string(":/images/file.svg").c_str()));
    }
    else
        fileItem->setIcon(QIcon(std::string(":/images/folder.svg").c_str()));
    QListWidget::addItem(fileItem);
}
