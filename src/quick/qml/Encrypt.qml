import QtQuick 2.0
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

Page {  
  background: Rectangle {
    color: "#2c3e50"
  }

  header: ToolBar {
    background: Rectangle {
      implicitHeight: 44
      color: "#2c3e50"
    }

    RowLayout {
      anchors.fill: parent

      ToolButton {
        id: pauseButton

        text: qsTr("Pause")
        icon.source: "qrc:/images/pauseIcon.png"
        icon.width: 12
        icon.height: 12
        display: AbstractButton.TextBesideIcon

        background: Rectangle {
          implicitHeight: 44
          radius: 3
          color: "#2980b9"
        }

        onClicked: {

        }
      }

      ToolButton {
        id: addFilesButton

        text: qsTr("Add file(s)")
        icon.source: "qrc:/images/addFilesIcon.png"
        icon.width: 12
        icon.height: 12
        display: AbstractButton.TextBesideIcon

        background: Rectangle {
          implicitHeight: 44
          radius: 3
          color: "#27ae60"
        }

        onClicked: {
          // Add files
        }
      }

      ToolButton {
        id: removeAllButton

        text: qsTr("Remove all")
        icon.source: "qrc:/images/clearFilesIcon.png"
        icon.width: 12
        icon.height: 12
        display: AbstractButton.TextBesideIcon

        background: Rectangle {
          implicitHeight: 44
          radius: 3
          color: "#c0392b"
        }

        onClicked: {

        }
      }

      ToolButton {
        id: settingsButton

        text: qsTr("Settings")
        icon.source: "qrc:/images/settingsIcon.png"
        icon.width: 12
        icon.height: 12
        display: AbstractButton.TextBesideIcon

        background: Rectangle {
          implicitHeight: 44
          radius: 3
          color: "#7f8c8d"
        }

        onClicked: {
          ui.navigate("Settings.qml");
        }
      }
    }
  }

  footer: ToolBar {
    background: Rectangle {
      implicitHeight: 44
      color: "#2c3e50"
    }

    RowLayout {
      anchors.fill: parent

      ToolButton {
        id: encryptButton

        text: qsTr("Encrypt")
        icon.source: "qrc:/images/lockIcon.png"
        icon.width: 12
        icon.height: 12
        display: AbstractButton.TextBesideIcon

        background: Rectangle {
          implicitHeight: 44
          radius: 3
          color: "#2980b9"
        }

        onClicked: {
//          ui.processFiles(, , true);
        }
      }

      ToolButton {
        id: decryptButton

        text: qsTr("Decrypt")
        icon.source: "qrc:/images/unlockIcon.png"
        icon.width: 12
        icon.height: 12
        display: AbstractButton.TextBesideIcon

        background: Rectangle {
          implicitHeight: 44
          radius: 3
          color: "#2980b9"
        }

        onClicked: {
//          ui.processFiles(, , false);
        }
      }
    }
  }

  ListModel {
    id: testFileModel

    ListElement {
      fileName: "C:\test.exe"
    }

    ListElement {
      fileName: "C:\test2.png"
    }

    ListElement {
      fileName: "C:\test3.dll"
    }
  }

  ColumnLayout {
    anchors.fill: parent

    ListView {
      model: testFileModel

      Layout.fillHeight: true
      Layout.fillWidth: true
      clip: true

      delegate: Label {
        text: fileName
      }
    }

    RowLayout {
      Label {
        text: qsTr("Password")
      }

      TextField {
        echoMode: TextInput.Password
      }
    }
  }
}
