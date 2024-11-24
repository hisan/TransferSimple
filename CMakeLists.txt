cmake_minimum_required(VERSION 3.16)

project(TransferSimpleServer VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_PREFIX_PATH $ENV{Qt6_DIR})
set(CMAKE_BUILD_TYPE Debug)

find_package(Qt6 REQUIRED COMPONENTS QuickControls2 Network)

qt_standard_project_setup()

if(WIN32)
    include_directories(
        D:\\Coding\\MSYS2_64\\mingw64\\include\\
    )
elseif(LINUX)
    include_directories(
        /usr/local/libevent/include/
        /usr/local/jsoncpp/include/
    )
endif()

if(WIN32)
    link_directories(
        D:\\Coding\\MSYS2_64\\mingw64\\lib
    )
elseif(LINUX)
    link_directories(
        /usr/local/libevent/lib/
        /usr/local/jsoncpp/lib/
    )
endif()

qt_add_resources(QML_RESOURCES qml.qrc)

qt_add_executable(${PROJECT_NAME}
    src/main.cpp
	src/fileHandle.cpp
	src/SecCodeGenerator.cpp
    ${QML_RESOURCES}
)

target_link_libraries(${PROJECT_NAME} 
    PRIVATE Qt6::Quick Qt6::QuickControls2 Qt6::Network
	pthread
	event
    jsoncpp
)