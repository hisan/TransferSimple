#include "FileListItem.hpp"

FileListItem::FileListItem(const std::string& fileName, const std::string& fileType, QListWidget* parent)
    : fileName_(fileName)
    , fileType_(fileType)
    , QListWidgetItem(parent)
{
    setText(fileName_.c_str());
}


bool FileListItem::isDir() const
{
    return fileType_ == "directory";
}


std::string FileListItem::getFileName() const
{
    return fileName_;
}