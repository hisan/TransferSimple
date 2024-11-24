#ifndef __MAIN_WINDOW_HPP__
#define __MAIN_WINDOW_HPP__

#include <QMainWindow>
#include "DownloadProgressWidget.hpp"
#include "ServerFileListWidget.hpp"
#include "ClientFileListWidget.hpp"
#include "NetworkConfigDialog.hpp"

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow() = default;

private slots:
    void showNetworkConfigDialog(bool);
    void switchOnLineStatus(bool isOnline);
    void handleServerStatus(QJsonDocument context, const QString &secCode);

private:
    void doLayout();

    QWidget* centralWidget_;

    QVBoxLayout *mainLayout_;
    QHBoxLayout *headerLayout_;
    QHBoxLayout *clientInputLinehlayout_;

    QPushButton *networkStatus_;
    QLabel *titleLabel_;
    QLabel* serverDirLabel_;
    QLineEdit* serverDirShowEdit_;
    ServerFileListWidget* serverListView_;

    QLabel* clientDirLabel_;
    QPushButton* selectedLocalDirButton_;
    QLineEdit* clientDirShowEdit_;
    ClientFileListWidget* clientListView_;

    QIcon networkOfflineIcon_;
    QIcon networkOnlineIcon_;
    QIcon downloadingIcon_;
    QIcon openLocalDirButtonIcon_;

    NetworkConfigDialog* networkConfigDialog_;
    DownloadProgressWidget* processWidget_;

    FileTransferHandler* fileTransferHandler_;
};

#endif//__MAIN_WINDOW_HPP__