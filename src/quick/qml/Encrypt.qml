import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Dialogs 1.3
import QtQuick.Layouts 1.13
import QtQuick.Controls.Material 2.2

Page {
  id: page

  header: ToolBar {
    implicitHeight: 44

    RowLayout {
      anchors.fill: parent

      Item {
        implicitHeight: 44
        implicitWidth: 44
      }

      Item {
        Layout.fillWidth: true
        Layout.fillHeight: true
      }

      Label {
        text: qsTr("Kryvo")
        font.pixelSize: 18
      }

      Item {
        Layout.fillWidth: true
        Layout.fillHeight: true
      }

      ToolButton {
        id: settingsButton

        implicitHeight: 44

        icon.source: "qrc:/images/settingsIcon.png"
        icon.width: 30
        icon.height: 30

        onClicked: {
          ui.navigate("Settings.qml");
        }
      }
    }
  }

  footer: ToolBar {
    implicitHeight: 44

    Material.primary: Material.Gray

    RowLayout {
      anchors.top: parent.top
      anchors.left: parent.left
      anchors.right: parent.right
      anchors.bottom: parent.bottom

      anchors.leftMargin: 5
      anchors.rightMargin: 5

      ToolButton {
        id: encryptButton

        implicitHeight: 44

        icon.source: "qrc:/images/lockIcon.png"
        icon.width: 30
        icon.height: 30

        Material.primary: Material.Blue

        onClicked: {
          ui.encryptFiles(passphraseTextField.text);
        }
      }

      Item {
        Layout.fillWidth: true
        Layout.fillHeight: true
      }

      ToolButton {
        id: decryptButton

        implicitHeight: 44

        icon.source: "qrc:/images/unlockIcon.png"
        icon.width: 30
        icon.height: 30

        onClicked: {
          ui.decryptFiles(passphraseTextField.text);
        }
      }

      Item {
        Layout.fillWidth: true
        Layout.fillHeight: true
      }

      ToolButton {
        id: pauseButton

        implicitHeight: 44
        checkable: true

        icon.source: checked ?
                     "qrc:/images/resumeIcon.png" :
                     "qrc:/images/pauseIcon.png"
        icon.width: 30
        icon.height: 30

        Material.accent: Material.theme == Material.Dark ? "#FFFFFF" : "#000000"

        onClicked: {
          ui.pauseProcessing(checked);
        }
      }

      Item {
        Layout.fillWidth: true
        Layout.fillHeight: true
      }

      ToolButton {
        id: removeAllButton

        implicitHeight: 44

        icon.source: "qrc:/images/clearFilesIcon.png"
        icon.width: 30
        icon.height: 30

        onClicked: {
          ui.removeFiles();
        }
      }

      Item {
        Layout.fillWidth: true
        Layout.fillHeight: true
      }

      ToolButton {
        id: addFilesButton

        implicitHeight: 44

        icon.source: "qrc:/images/addFilesIcon.png"
        icon.width: 30
        icon.height: 30

        onClicked: {
//          ui.navigate("SelectFile.qml");
          inputFileDialog.visible = true;
        }
      }
    }
  }

  ColumnLayout {
    anchors.fill: parent

    ListView {
      property alias windowWidth: page.width

      model: inputFiles
      clip: true

      Layout.fillHeight: true
      Layout.fillWidth: true
      Layout.leftMargin: 8
      Layout.rightMargin: 8

      delegate: RowLayout {
        id: row
        spacing: 6

        property var item: model.modelData ? model.modelData : model

        Label {
          text: item.fileName
          elide: Text.ElideLeft
          font.pixelSize: 18
          Layout.preferredWidth: {
            row.ListView.view.windowWidth * 2 / 3 - 9 > 0 ?
            row.ListView.view.windowWidth * 2 / 3 - 9 :
            9
          }
        }

        Label {
          text: item.task
          elide: Text.ElideRight
          font.pixelSize: 18
          Layout.preferredWidth: {
            row.ListView.view.windowWidth / 6 - 9 > 0 ?
            row.ListView.view.windowWidth / 6 - 9 :
            9
          }
        }

        ProgressBar {
          from: 0
          to: 100
          value: item.progress
          Layout.preferredWidth: {
            row.ListView.view.windowWidth / 6 - 9 > 0 ?
            row.ListView.view.windowWidth / 6 - 9 :
            9
          }
        }
      }
    }

    RowLayout {
      spacing: 10

      Layout.leftMargin: 10
      Layout.rightMargin: 10

      Label {
        text: qsTr("Output path")
        font.pixelSize: 18
        Material.primary: "#2C3E50"
      }

      TextField {
        id: outputTextField
        text: ui.outputPathString
        readOnly: true

        font.pixelSize: 18
        Layout.fillWidth: true
        Material.background: "#2C3E50"

        onPressed: {
          outputFileDialog.folder = Qt.resolvedUrl(ui.outputPath);

          outputFileDialog.open();
        }
      }
    }

    RowLayout {
      spacing: 10

      Layout.leftMargin: 10
      Layout.rightMargin: 10

      Label {
        text: qsTr("Password")
        font.pixelSize: 18
        Material.primary: "#2C3E50"
      }

      TextField {
        id: passphraseTextField
        echoMode: TextInput.Password
        font.pixelSize: 18
        Layout.fillWidth: true
        Material.background: "#2C3E50"
      }
    }

    Item {
      Layout.bottomMargin: 10
    }
  }

  FileDialog {
    id: inputFileDialog
    title: qsTr("Please choose files")
    selectExisting: true
    selectMultiple: true
    selectFolder: false

    onAccepted: {
      ui.addFiles(inputFileDialog.fileUrls);

      close();
    }

    onRejected: {
      close();
    }
  }

  FileDialog {
    id: outputFileDialog
    title: qsTr("Please choose an output directory")
    selectExisting: true
    selectFolder: true
    selectMultiple: false

    onAccepted: {
      ui.updateOutputPath(folder);
      close();
    }

    onRejected: {
      close();
    }
  }
}
