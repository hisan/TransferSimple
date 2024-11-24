# use vcpkg to do the c++ package manangement
https://learn.microsoft.com/en-us/vcpkg/get_started/get-started?pivots=shell-powershell

$env:VCPKG_ROOT = "D:\myPrograms\vcpkg"
$env:PATH = "$env:VCPKG_ROOT;$env:PATH"

## cmake + Qt online + vcpkg
https://stackoverflow.com/questions/78090193/vcpkg-qt-cmake-visual-studio-2019

~~~sh
cmake_minimum_required (VERSION 3.8)

set(CMAKE_AUTOMOC ON)  # https://stackoverflow.com/questions/14170770/unresolved-external-symbol-public-virtual-struct-qmetaobject-const-thiscal
set(CMAKE_INCLUDE_CURRENT_DIR ON)  # so that mainwindow.h and mainwindow.cpp is seen

project (HelloWorld)

find_package(fmt CONFIG REQUIRED)

find_package(Qt6Core REQUIRED)
find_package(Qt6Gui CONFIG REQUIRED)
find_package(Qt6Widgets REQUIRED)

add_executable (HelloWorld "helloworld.cpp" "helloworld.h" "mainwindow.cpp")
#add_executable (HelloWorld "mainwindow.cpp" "mainwindow.h" "mainwindow.cpp")

target_link_libraries(HelloWorld PRIVATE fmt::fmt)
target_link_libraries(HelloWorld PRIVATE Qt::Gui)
target_link_libraries(HelloWorld PRIVATE Qt::Core)
target_link_libraries(HelloWorld PRIVATE Qt::Widgets)
~~~


set http_proxy=http://127.0.0.1:4780
set https_proxy=http://127.0.0.1:4780

D:\myPrograms\vcpkg\downloads\tools\cmake-3.30.1-windows\cmake-3.30.1-windows-i386\bin\cmake.exe .. -G "Visual Studio 17 2022" -A x64

D:\myPrograms\vcpkg\downloads\tools\cmake-3.30.1-windows\cmake-3.30.1-windows-i386\bin\cmake.exe .. -G "Visual Studio 17 2022" -A x64