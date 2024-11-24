#ifndef __NETWORK_CONFIG_DIALOG_HPP__
#define __NETWORK_CONFIG_DIALOG_HPP__

#include <memory>
#include <iostream>
#include <QJsonArray>
#include <QString>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QStringListModel>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>

class NetworkConfigDialog : public QDialog
{
    Q_OBJECT
public:
    NetworkConfigDialog(QWidget *parent = nullptr);

    std::string getIp();
    std::string getPort();
    std::string getSecCode();
    
    bool isOnline() {return isOnline_.load();}

signals:
    void connectedServer(QJsonDocument context, const QString &secCode);
    void connectFailed();

public slots:
    void handleWhenRequestFinished();

private slots:
    void refreshClientDilog(const QVariantMap &response, const QString &secCode);
    void connectToServer();


private:
    void initUi();

    QVBoxLayout* mainLayout_;
    QHBoxLayout* ipLayout_;
    QHBoxLayout* portLayout_;
    QHBoxLayout* securityLayout_;

    QLabel* ipLabel_;
    QLabel* portLabel_;
    QLabel* securityLabel_;

    QLineEdit* ipInput_;
    QLineEdit* portInput_;
    QLineEdit* secCodeInput_;

    QPushButton *connectButton_;

    std::shared_ptr<QNetworkReply> reply_;
    std::atomic<bool> isOnline_;
};


#endif//__NETWORK_CONFIG_DIALOG_HPP__