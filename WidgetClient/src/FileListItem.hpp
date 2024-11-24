#ifndef __FILE_LIST_ITEM_HPP__
#define __FILE_LIST_ITEM_HPP__

#include <QListWidget>
#include <QListWidgetItem>

class FileListItem: public QListWidgetItem
{
public:
    FileListItem(const std::string& fileName, const std::string& fileType, QListWidget* parent = nullptr);
    bool isDir() const;
    std::string getFileName() const;
private:
    std::string fileType_;
    std::string fileName_;
};

#endif//__FILE_LIST_ITEM_HPP__
