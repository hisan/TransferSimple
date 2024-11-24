cmake_minimum_required(VERSION 3.16)

project(TransferEasy VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_PREFIX_PATH $ENV{Qt6_DIR})
set(CMAKE_BUILD_TYPE Debug)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network Gui Concurrent Svg)

qt_standard_project_setup()
qt_add_resources(RESOURCES resource.qrc)

set(SOURCES
    src/DownloadProgressWidget.hpp
    src/DownloadProgressWidget.cpp
    src/FileTransferHandler.hpp
    src/FileTransferHandler.cpp
    src/DirectoryHandler.hpp
    src/DirectoryHandler.cpp
    src/FileListWidget.hpp
    src/FileListWidget.cpp
    src/FileListItem.hpp
    src/FileListItem.cpp
    src/ServerFileListWidget.cpp
    src/ServerFileListWidget.hpp
    src/ClientFileListWidget.cpp
    src/ClientFileListWidget.hpp
    src/NetworkConfigDialog.hpp
    src/NetworkConfigDialog.cpp
    src/HttpSessionManager.hpp
    src/HttpSessionManager.cpp
    src/MainWindow.hpp
    src/MainWindow.cpp
    src/main.cpp
    ${RESOURCES}
)

qt_add_executable(${PROJECT_NAME} ${SOURCES})

set_property(TARGET ${PROJECT_NAME} PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")

target_link_libraries(${PROJECT_NAME}
	PRIVATE Qt6::Core 
            Qt6::Widgets 
            Qt6::Network 
            Qt6::Gui 
            Qt6::Concurrent
            Qt6::Svg
)