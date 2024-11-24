#include <QDir>
#include <QFileInfoList>
#include "DirectoryHandler.hpp"

DirectoryHandler::DirectoryHandler()
{
}

bool DirectoryHandler::validateAndLoadDirectory(const std::string& path)
{
    QDir dir(path.c_str());
    if (dir.exists())
    {
        curPath_ = path;
        return true;
    }
    return false;
}

std::vector<std::tuple<std::string, std::string>> DirectoryHandler::loadDirectory(const std::string& path)
{
    std::vector<std::tuple<std::string, std::string>> files;
    QDir dir(path.c_str());
    if (dir.exists()) {
        QFileInfoList list = dir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot);
        foreach (QFileInfo fileInfo, list) {
            files.push_back(
                std::make_tuple(fileInfo.fileName().toStdString(), fileInfo.isDir() ? "directory" : "file" ));
        }
    }
    return files;
}
