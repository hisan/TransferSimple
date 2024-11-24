#ifndef __FILE_LIST_WIDGET_HPP__
#define __FILE_LIST_WIDGET_HPP__

#include <QListWidget>

class FileListWidget: public QListWidget
{
    Q_OBJECT
public:
    FileListWidget(QWidget* parent = nullptr);
    void addItem(const std::string& fileName, const std::string& fileType);

public slots:
    virtual void handleWhenItemDoubleClicked(QListWidgetItem* clickedItem) = 0;

private:
    const std::string iconPath_;
};

#endif//__FILE_LIST_WIDGET_HPP__
