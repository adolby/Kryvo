import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts

Page {
  header: ToolBar {
    implicitHeight: 44

    RowLayout {
      anchors.fill: parent

      ToolButton {
        id: backButton

        implicitHeight: 44

        icon.source: "qrc:/images/backIcon.png"
        icon.width: 30
        icon.height: 30

        onClicked: {
          ui.navigateBack();
        }

        Layout.rightMargin: 5
      }

      Item {
        Layout.fillWidth: true
      }

      Label {
        text: qsTr("Settings")
        font.pixelSize: 18
      }

      Item {
        Layout.fillWidth: true
      }

      Item {
        implicitHeight: 44
        implicitWidth: 44
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
          text: qsTr("Cryptography provider")
        }

        ComboBox {
          model: ["Botan", "OpenSsl"]

          onActivated: {
            ui.updateCryptoProvider(currentText);
          }

          Component.onCompleted: {
            currentIndex = find(ui.cryptoProvider);
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
          model: ["256", "128"]

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
          text: qsTr("Compression format")
        }

        ComboBox {
          model: ["gzip", "None"]

          onActivated: {
            ui.updateCompressionFormat(currentText);
          }

          Component.onCompleted: {
            currentIndex = find(ui.compressionFormat);
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
