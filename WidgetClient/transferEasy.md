@startuml
component Gui {
    class ProcessWidget {
        + addNewTask
        + removeTask
    }

    class MainWindow {
        - NetworkWorker* worker
        - QThread* workerThread
        - QLabel* infoLabel
        + makeFooRequest()
        + makeBarRequest()
        + onDataReceived(const QByteArray &data)
        + updateInfoLabel(const QString &info)
    }

    MainWindow -> NetworkWorker : uses
}

component Networker {

    class NetworkWorker {
        - QNetworkAccessManager* manager
        + makeRequest(const QUrl &url)
        + onFinished()
        ~ dataReceived(const QByteArray &data)
        ~ updateGUI(const QString &info)
        ~ finished()
    }

    class QNetworkAccessManager {
        + get(const QNetworkRequest &request) : QNetworkReply*
    }

    class QNetworkReply {
        + readAll() : QByteArray
        + deleteLater()
    }

    NetworkWorker -> QNetworkAccessManager : uses
    QNetworkAccessManager -> QNetworkReply : creates
    QNetworkReply -> NetworkWorker : notifies
}
@enduml