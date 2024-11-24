#include <string>
#include <QFileDialog>
#include <QStringList>
#include "MainWindow.hpp"
#include <QStandardPaths>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowIcon(QIcon(":/images/transfer.png"));
    centralWidget_ = new QWidget(this);

    networkConfigDialog_ = new NetworkConfigDialog(nullptr);
    connect(networkConfigDialog_, &NetworkConfigDialog::connectedServer, this, &MainWindow::handleServerStatus);
    connect(networkConfigDialog_, &NetworkConfigDialog::connectFailed, this, [this](){
        this->switchOnLineStatus(false);
    });

    networkOfflineIcon_ = QIcon(std::string(":/images/network-offline.svg").c_str());
    networkOnlineIcon_ = QIcon(std::string(":/images/network-online.svg").c_str());
    downloadingIcon_ = QIcon(std::string(":/images/downloadingIcon.png").c_str());
    openLocalDirButtonIcon_ = QIcon(std::string(":/images/folder.svg").c_str());

    networkStatus_ = new QPushButton(this);
    networkStatus_->setFixedSize(40, 40);
    networkStatus_->setIcon(networkOfflineIcon_);
    networkStatus_->setIconSize(networkStatus_->size());
    networkStatus_->setStyleSheet("QPushButton {"
                        "border:none;"
                        "background: transparent;"
                        "}");
    connect(networkStatus_, &QPushButton::clicked, this, &MainWindow::showNetworkConfigDialog);

    titleLabel_ = new QLabel("TransferEasy");
    titleLabel_->setAlignment(Qt::AlignCenter);
    titleLabel_->setStyleSheet("QLabel {"
        "font-size: 40px;"
        "background-color: #85C1E9"
    "}");
   
    serverDirLabel_ = new QLabel("Server Dir");
    serverDirLabel_->setAlignment(Qt::AlignCenter);
    serverDirLabel_->setStyleSheet("QLabel {"
        "font-size: 20px;"
        "background-color: #D1F2EB"
    "}");
    serverDirShowEdit_ = new QLineEdit(this);

    clientDirShowEdit_ = new QLineEdit(this);
    QString appPrivateDir = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    clientDirShowEdit_->setText(appPrivateDir);
    QObject::connect(clientDirShowEdit_, &QLineEdit::textChanged, [](const QString& text){});

    processWidget_ = new DownloadProgressWidget(nullptr);
    fileTransferHandler_ = new FileTransferHandler(processWidget_, networkConfigDialog_, nullptr);

    clientListView_ = new ClientFileListWidget(fileTransferHandler_, clientDirShowEdit_, this);
    connect(fileTransferHandler_, &FileTransferHandler::downloadComplete, clientListView_,
        &ClientFileListWidget::refreshLocalFileItem);

    serverListView_ = new ServerFileListWidget(
        fileTransferHandler_,
        clientDirShowEdit_,
        this);

    clientDirLabel_ = new QLabel("Local Dir");
    clientDirLabel_->setAlignment(Qt::AlignCenter);
    clientDirLabel_->setStyleSheet("QLabel {"
        "font-size: 20px;"
        "background-color: #D1F2EB"
    "}");

    selectedLocalDirButton_ = new QPushButton();
    selectedLocalDirButton_->setFixedSize(40, 40);
    selectedLocalDirButton_->setIcon(openLocalDirButtonIcon_);
    selectedLocalDirButton_->setIconSize(selectedLocalDirButton_->size());
    selectedLocalDirButton_->setStyleSheet("QPushButton {"
        "border: none;"
        "background: transparent;"
        "}");
    connect(selectedLocalDirButton_, &QPushButton::clicked, [this](bool) {
        processWidget_->show();
    });

    auto freshDownStatusIconFunc = [this]() {
        if (this->processWidget_->hasOngoingTask()) {
            this->selectedLocalDirButton_->setIcon(this->downloadingIcon_);
        } else {
            this->selectedLocalDirButton_->setIcon(this->openLocalDirButtonIcon_);
        }
    };

    connect(processWidget_, &DownloadProgressWidget::oneItemAdded, freshDownStatusIconFunc);
    connect(processWidget_, &DownloadProgressWidget::oneItemFinished, freshDownStatusIconFunc);

    doLayout();
    setCentralWidget(centralWidget_);
}

void MainWindow::doLayout()
{
    mainLayout_ = new QVBoxLayout(centralWidget_);
    headerLayout_ = new QHBoxLayout();
    headerLayout_->addWidget(networkStatus_);
    headerLayout_->addWidget(titleLabel_);

    mainLayout_->addLayout(headerLayout_);
    mainLayout_->addWidget(serverDirLabel_);
    mainLayout_->addWidget(serverDirShowEdit_);
    mainLayout_->addWidget(serverListView_);
    mainLayout_->addWidget(clientDirLabel_);

    clientInputLinehlayout_ = new QHBoxLayout();
    clientInputLinehlayout_->addWidget(selectedLocalDirButton_);
    clientInputLinehlayout_->addWidget(clientDirShowEdit_);
    mainLayout_->addLayout(clientInputLinehlayout_);
    mainLayout_->addWidget(clientListView_);
}

void MainWindow::showNetworkConfigDialog(bool)
{
    networkConfigDialog_->exec();
}

void MainWindow::switchOnLineStatus(bool isOnline)
{
    if (isOnline) {
        networkStatus_->setIcon(networkOnlineIcon_);
    }
    else {
        networkStatus_->setIcon(networkOfflineIcon_);
    }
    networkStatus_->setIconSize(networkStatus_->size());
    networkConfigDialog_->hide();
}

void MainWindow::handleServerStatus(QJsonDocument context, const QString &secCode)
{
    switchOnLineStatus(true);
    std::cout << __FUNCTION__ << ", received response\n";
    auto qJsonStr = QString(context.toJson(QJsonDocument::Indented));

    std::cout << "json: " << qJsonStr.toStdString() << "\n";

    QJsonObject childrenListObj = context.object();
    auto newServerDirStr = childrenListObj["dir"].toString();
    QJsonArray childrenArray = childrenListObj["children"].toArray();

    serverListView_->clear();
    serverDirShowEdit_->setText(newServerDirStr);

    std::unordered_map<std::string, std::uint64_t> fileInfo;

    for (const auto& value : childrenArray)
    {
        auto fileItemObj = value.toObject();
        auto fileType = fileItemObj["fileType"].toString().toStdString();
        auto fileName = fileItemObj["fileName"].toString().toStdString();
        if (fileType != "file" && fileType != "directory")
            continue;
        if (fileType == "file") {
            std::uint64_t size = (std::size_t)fileItemObj["fileSize"].toInteger();
            fileInfo.insert({fileName, size});
        }
        serverListView_->addItem(fileName, fileType);
        std::cout << "key: " << fileType << ", value: " << fileName << "\n"; 
    }

    processWidget_->freshFileInfo(fileInfo);
}