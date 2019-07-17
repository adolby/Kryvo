import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import QtQuick.Controls.Material 2.2

Page {
  header: ToolBar {
    implicitHeight: 44

    RowLayout {
      anchors.fill: parent

      Label {
        text: qsTr("Settings")

        font.pixelSize: 18
        Layout.alignment: Qt.AlignCenter
      }
    }
  }

  footer: ToolBar {
    implicitHeight: 44

    RowLayout {
      anchors.fill: parent

      Item {
        Layout.fillWidth: true
      }

      ToolButton {
        id: backButton

        implicitHeight: 44

        icon.source: "qrc:/images/backIcon.png"
        icon.width: 30
        icon.height: 30

        onClicked: {
          ui.navigate("Encrypt.qml");
        }

        Layout.rightMargin: 5
      }
    }
  }

  Flickable {
    anchors.fill: parent
    contentHeight: containerLayout.implicitHeight

    ColumnLayout {
      id: containerLayout
      anchors.fill: parent

      RowLayout {
        height: 50
        spacing: 15

        Layout.leftMargin: 10

        Label {
          id: cipherLabel
          font.pixelSize: 18
          text: qsTr("Cipher")
        }

        ComboBox {
          model: ["AES", "Serpent"]

          onActivated: {
            ui.updateCipher(currentText);
          }

          Component.onCompleted: {
            currentIndex = find(ui.cipher);
          }
        }
      }

      Rectangle {
        width: parent.width
        height: 1
        color: "white"
      }

      RowLayout {
        height: 50
        spacing: 15

        Layout.leftMargin: 10

        Label {
          id: keySizeLabel
          font.pixelSize: 18
          text: qsTr("Key size (bits)")
        }

        ComboBox {
          model: ["128", "192", "256"]

          onActivated: {
            ui.updateKeySize(currentText);
          }

          Component.onCompleted: {
            currentIndex = find(ui.keySize);
          }
        }
      }

      Rectangle {
        width: parent.width
        height: 1
        color: "white"
      }

      RowLayout {
        height: 50
        spacing: 15

        Layout.leftMargin: 10

        Label {
          id: modeLabel
          font.pixelSize: 18
          text: qsTr("Mode")
        }

        ComboBox {
          model: ["GCM", "EAX"]

          onActivated: {
            ui.updateModeOfOperation(currentText);
          }

          Component.onCompleted: {
            currentIndex = find(ui.modeOfOperation);
          }
        }
      }

      Rectangle {
        width: parent.width
        height: 1
        color: "white"
      }

      RowLayout {
        height: 50
        spacing: 15

        Layout.leftMargin: 10

        Label {
          id: compressFilesLabel
          font.pixelSize: 18
          text: qsTr("Compress files before encryption")
        }

        Switch {
          checked: ui.compressionMode

          onCheckedChanged: {
            ui.updateCompressionMode(checked);
          }
        }
      }

      Rectangle {
        width: parent.width
        height: 1
        color: "white"
      }

      RowLayout {
        height: 50
        spacing: 15

        Layout.leftMargin: 10

        Label {
          id: deleteIntermediateFilesLabel
          font.pixelSize: 18
          text: qsTr("Delete intermediate files")
        }

        Switch {
          checked: ui.removeIntermediateFiles

          onCheckedChanged: {
            ui.updateRemoveIntermediateFiles(checked);
          }
        }
      }

      Rectangle {
        width: parent.width
        height: 1
        color: "white"
      }
    }
  }
}
