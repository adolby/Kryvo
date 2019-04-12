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

        contentItem: Label {
          text: pauseButton.text
          font: pauseButton.font
          color: "#ffffff"
          horizontalAlignment: Text.AlignHCenter
          verticalAlignment: Text.AlignVCenter
          elide: Text.ElideRight
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

        contentItem: Label {
          text: addFilesButton.text
          font: addFilesButton.font
          color: "#ffffff"
          horizontalAlignment: Text.AlignHCenter
          verticalAlignment: Text.AlignVCenter
          elide: Text.ElideRight
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

        contentItem: Label {
          text: removeAllButton.text
          font: removeAllButton.font
          color: "#ffffff"
          horizontalAlignment: Text.AlignHCenter
          verticalAlignment: Text.AlignVCenter
          elide: Text.ElideRight
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

        contentItem: Label {
          text: settingsButton.text
          font: settingsButton.font
          color: "#ffffff"
          horizontalAlignment: Text.AlignHCenter
          verticalAlignment: Text.AlignVCenter
          elide: Text.ElideRight
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

        contentItem: Label {
          text: encryptButton.text
          font: encryptButton.font
          color: "#ffffff"
          horizontalAlignment: Text.AlignHCenter
          verticalAlignment: Text.AlignVCenter
          elide: Text.ElideRight
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

        contentItem: Label {
          text: decryptButton.text
          font: decryptButton.font
          color: "#ffffff"
          horizontalAlignment: Text.AlignHCenter
          verticalAlignment: Text.AlignVCenter
          elide: Text.ElideRight
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

  contentItem: Item {
    ColumnLayout {
      ListView {
        model: testFileModel

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
}
