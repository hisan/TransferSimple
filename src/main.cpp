#include <thread>

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "fileHandle.hpp"
#include "SecCodeGenerator.hpp"

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
#endif


int main(int argc, char *argv[])
{
    #if 1
    std::string ipAddr = "0.0.0.0";
    uint16_t port = 8182;

    auto codeGeneratorHandler_ = std::make_shared<SecCodeGenerator>();
    auto fileHandler_ = std::make_shared<FileHandler>(codeGeneratorHandler_, ipAddr, port);

    #ifdef __WIN32
        WSADATA wsaData;
        int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
        assert(result == 0);
    #endif

    std::thread httpServer([&fileHandler_]() {
        fileHandler_->init();
    });
    httpServer.detach();
    #endif


    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("SecCodeGenerator", codeGeneratorHandler_.get());

    const QUrl url(QStringLiteral("qrc:/src/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    engine.load(url);

    app.exec();
    return 0;
}
