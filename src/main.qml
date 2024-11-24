import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Qt.labs.platform 1.1 as Platform
import QtQuick.Dialogs

ApplicationWindow {
    visible: true
    width: 300
    height: 400
    title: "TransferEasy"
    property string selectedDir: ""

    Column {
        id: pre
        width: parent.width
        height: parent.height
        spacing: 10

        // First Part: Header
        Row {
            id: firstPart
            width: parent.width
            height: parent.height * 0.1

            Rectangle {
                id: titleBar
                width: parent.width
                height: parent.height
                color: "lightblue"

                Text {
                    anchors.centerIn: titleBar
                    text: qsTr("TransferEasy")
                    font.pixelSize: 20
                    horizontalAlignment: Text.AlignHCenter  // Center align text horizontally
                    verticalAlignment: Text.AlignVCenter  // Center align text vertically
                }
            }
        }

        // Second Part: Directory Setting
        Row {
            id: secondPart
            width: parent.width
            height: parent.height * 0.1
            spacing: 10
            padding: 10

            Rectangle {
                id: dirSetting
                width: parent.width * 0.2
                height: parent.height
                color: "lightgray"
                radius: 5

                MouseArea {
                    anchors.fill: parent
                    onClicked: folderDialog.open()
                }

                Text {
                    anchors.centerIn: parent
                    text: qsTr("存储目录")
                    font.pixelSize: 13
                    verticalAlignment: Text.AlignVCenter
                }
            }

            TextField {
                id: dirPath
                width: parent.width - dirSetting.width - 2*parent.spacing - padding
                height: parent.height
                text: selectedDir
                readOnly: true
                placeholderText: qsTr("Select a directory")
            }

            FolderDialog {
                id: folderDialog
                title: qsTr("Select Directory")
                currentFolder: Platform.StandardPaths.writableLocation(Platform.StandardPaths.DocumentsLocation)
                onAccepted: {
                    dirPath.text = urlFormatting(folderDialog.selectedFolder.toString())
                    secCodeShowArea.text = SecCodeGenerator.generateSecCode(dirPath.text)
                }
            }
        }

        // Third Part: Numeric Display
        Rectangle {
            id: thirdPart
            anchors.top: secondPart.bottom + padding
            width: parent.width
            height: parent.height - secondPart.height - firstPart.height
            color: "lightgray"

            Text {
                id: secCodeShowArea
                anchors.centerIn: parent
                text: ""
                font.pixelSize: 100
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
            }
        }
    }

    function urlFormatting(urlFoler) {
        return urlFoler.replace(/^file:\/\//, '')
    }
}