import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import QtQuick.Dialogs

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
      }

      Label {
        text: qsTr("Kryvo")
        font.pixelSize: 18
      }

      Item {
        Layout.fillWidth: true
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
      }

      ToolButton {
        id: removeAllButton

        implicitHeight: 44

        icon.source: "qrc:/images/clearFilesIcon.png"
        icon.width: 30
        icon.height: 30

        onClicked: {
          pauseButton.checked = false;
          ui.pauseProcessing(false);
          ui.removeFiles();
        }
      }

      Item {
        Layout.fillWidth: true
      }

      ToolButton {
        id: addFilesButton

        implicitHeight: 44

        icon.source: "qrc:/images/addFilesIcon.png"
        icon.width: 30
        icon.height: 30

        onClicked: {
          if (Qt.platform.os === "android" || Qt.platform.os === "ios") {
            ui.navigate("FileSelector.qml",
                        {"folder": Qt.resolvedUrl(ui.inputPath),
                         "selectFolder": false});
          } else {
            inputFileDialog.open();
          }
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
            row.ListView.view.windowWidth / 2 - 9 > 0 ?
            row.ListView.view.windowWidth / 2 - 9 :
            9
          }
        }

        Label {
          text: item.task
          elide: Text.ElideRight
          font.pixelSize: 18
          Layout.preferredWidth: {
            row.ListView.view.windowWidth * 3 / 8 - 9 > 0 ?
            row.ListView.view.windowWidth * 3 / 8 - 9 :
            9
          }
        }

        ProgressBar {
          from: 0
          to: 100
          value: item.progress
          Layout.preferredWidth: {
            row.ListView.view.windowWidth / 8 - 9 > 0 ?
            row.ListView.view.windowWidth / 8 - 9 :
            9
          }
        }
      }
    }

    RowLayout {
      ToolButton {
        id: leftButton

        implicitHeight: 44

        icon.source: "qrc:/images/leftArrowIcon.png"
        icon.width: 30
        icon.height: 30

        onClicked: {
            ui.navigateMessageLeft();
        }
      }

      Item {
        Layout.fillWidth: true

        Label {
          text: ui.statusMessage
          elide: Text.ElideLeft
          font.pixelSize: 18
          width: parent.width
          anchors.centerIn: parent
          Material.primary: "#2C3E50"
        }
      }

      ToolButton {
        id: rightButton

        implicitHeight: 44

        icon.source: "qrc:/images/rightArrowIcon.png"
        icon.width: 30
        icon.height: 30

        onClicked: {
            ui.navigateMessageRight();
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
          if (Qt.platform.os === "android" || Qt.platform.os == "ios") {
            ui.navigate("FileSelector.qml",
                        {"folder": Qt.resolvedUrl(ui.outputPath),
                         "selectFolder": true});
          } else {
            outputDirDialog.open();
          }
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
        text: ui.password
        echoMode: TextInput.Password
        font.pixelSize: 18
        Layout.fillWidth: true
        Material.background: "#2C3E50"

        onTextChanged: {
          ui.updatePassword(text);
        }
      }
    }

    Item {
      Layout.bottomMargin: 10
    }
  }

  FileDialog {
    id: inputFileDialog
    title: qsTr("Please choose files")
    currentFolder: Qt.resolvedUrl(ui.outputPath)
    fileMode: FileDialog.OpenFiles

    onAccepted: {
      ui.addFiles(inputFileDialog.selectedFiles);
      close();
    }

    onRejected: {
      close();
    }
  }

  FolderDialog {
    id: outputDirDialog
    title: qsTr("Please choose an output directory")
    currentFolder: Qt.resolvedUrl(ui.outputPath)

    onAccepted: {
      ui.updateOutputPath(outputDirDialog.selectedFolder);
      close();
    }

    onRejected: {
      close();
    }
  }
}
