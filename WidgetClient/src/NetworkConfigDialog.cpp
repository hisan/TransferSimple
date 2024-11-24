#include "NetworkConfigDialog.hpp"

NetworkConfigDialog::NetworkConfigDialog(QWidget *parent)
    : isOnline_(false)
    , QDialog(parent)
{
    initUi();
}

std::string NetworkConfigDialog::getIp()
{
    return ipInput_->text().toStdString();
}

std::string NetworkConfigDialog::getPort()
{
    return portInput_->text().toStdString();
}

std::string NetworkConfigDialog::getSecCode()
{
    return secCodeInput_->text().toStdString();
}

void NetworkConfigDialog::initUi()
{
    setFixedSize(300, 200);
    mainLayout_ = new QVBoxLayout(this);
    ipLayout_ = new QHBoxLayout();

    ipLabel_ = new QLabel("Server IP:");
    ipLabel_->setFixedWidth(100);

    ipInput_ = new QLineEdit();
    ipInput_->setPlaceholderText("e.g. 127.0.0.1");

    ipInput_->setText("192.168.2.104");

    ipLayout_->addWidget(ipLabel_);
    ipLayout_->addWidget(ipInput_);
    mainLayout_->addLayout(ipLayout_);

    portLayout_ = new QHBoxLayout();
    portLabel_ = new QLabel("Port:");
    portLabel_->setFixedWidth(100);
    portInput_ = new QLineEdit();
    portInput_->setPlaceholderText("e.g. 1024");
    
    portInput_->setText("8182");

    portLayout_->addWidget(portLabel_);
    portLayout_->addWidget(portInput_);
    mainLayout_->addLayout(portLayout_);

    securityLayout_ = new QHBoxLayout();
    securityLabel_ = new QLabel("Security Code:");
    securityLabel_->setFixedWidth(100);
    secCodeInput_ = new QLineEdit();
    secCodeInput_->setPlaceholderText("4-digit code");
    securityLayout_->addWidget(securityLabel_);
    securityLayout_->addWidget(secCodeInput_);
    mainLayout_->addLayout(securityLayout_);

    QPushButton *connectButton_ = new QPushButton("Connect");
    mainLayout_->addWidget(connectButton_);

    connect(connectButton_, &QPushButton::clicked, this, &NetworkConfigDialog::connectToServer);
    setLayout(mainLayout_);
}

void NetworkConfigDialog::refreshClientDilog(const QVariantMap &response, const QString &secCode)
{
    std::cout << __FUNCTION__ << ", received response\n";
}

void NetworkConfigDialog::connectToServer()
{
    if (ipInput_->text().isEmpty() || portInput_->text().isEmpty() || secCodeInput_->text().isEmpty()) {
        QMessageBox::warning(this, "Invalid Input", "Please fill all fields with valid input.");
        emit connectFailed();
        return;
    }

    QString url = QString("http://%1:%2/getCacheDir")
        .arg(ipInput_->text())
        .arg(portInput_->text());

    QJsonObject json;
    json["secCode"] =  secCodeInput_->text();

    QNetworkAccessManager* manager = new QNetworkAccessManager(this);
    QUrl urlObj(url);
    QNetworkRequest request({urlObj});
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    reply_.reset(manager->post(request, QJsonDocument(json).toJson()));
    connect(reply_.get(), &QNetworkReply::finished, this, &NetworkConfigDialog::handleWhenRequestFinished);
}

void NetworkConfigDialog::handleWhenRequestFinished()
{
    std::cout << "received response: \n";
    if (reply_->error() == QNetworkReply::NoError)\
    {
        QJsonDocument responseDoc = QJsonDocument::fromJson(reply_->readAll());
        isOnline_.store(true);
        emit connectedServer(responseDoc, secCodeInput_->text());
    }
    else
    {
        QMessageBox::critical(this, "Connection Failed", "Unable to connect to server.");
        isOnline_.store(false);
        emit connectFailed();
    }
}