import QtQuick 2.13
import QtQuick.Controls 2.13
import QtQuick.Controls.Material 2.13
import QtQuick.Layouts 1.13
import Qt.labs.folderlistmodel 2.13

Page {
  id: fileSelector
  property url folder
  property bool selectFolder: false

  header: ToolBar {
    implicitHeight: 44

    RowLayout {
      anchors.fill: parent

      ToolButton {
        implicitHeight: 44

        icon.source: "qrc:/images/backIcon.png"
        icon.width: 30
        icon.height: 30

        onClicked: {
          ui.navigateBack();
        }
      }

      Item {
        Layout.fillWidth: true
        Layout.fillHeight: true
      }

      Label {
        text: fileSelector.folder
        font.pixelSize: 18
      }

      Item {
        Layout.fillWidth: true
        Layout.fillHeight: true
      }

      Item {
        implicitHeight: 44
        implicitWidth: 44
      }
    }
  }

  FolderListModel {
    id: folderModel
    folder: fileSelector.folder
    showDirsFirst: true
    showHidden: true
    showFiles: !fileSelector.selectFolder
  }

  Component {
    id: fileRowDelegate

    Item {
      id: row
      property var item: model.modelData ? model.modelData : model
      width: parent.width
      height: childrenRect.height

      Image {
        id: icon
        source: item.fileIsDir ?
                "qrc:/images/dirclosed-128.png" :
                "qrc:/images/file-128.png"
        height: fileLabel.height
        width: height
        anchors.left: parent.left
        anchors.leftMargin: 8
      }

      Label {
        id: fileLabel
        text: item.fileName
        elide: Text.ElideLeft
        font.pixelSize: 24
        anchors.left: icon.visible ? icon.right : parent.left
        anchors.leftMargin: 8
      }

      MouseArea {
        anchors.fill: parent

        onClicked: {
          if (item.fileIsDir) {
            fileSelector.folder = item.fileURL;
          } else {
            ui.addFile(item.fileURL);
            ui.changePage("Encrypt.qml");
          }
        }
      }
    }
  }

  Component {
    id: fileToolBar

    ToolBar {
      width: parent.width
      z: 2

      RowLayout {
        anchors.fill: parent

        Item {
          Layout.fillWidth: true
          Layout.fillHeight: true
        }

        ToolButton {
          id: upButton
          implicitHeight: 44

          icon.source: "qrc:/images/up-128.png"
          icon.width: 30
          icon.height: 30

          onClicked: {
            if (folderModel.parentFolder.toString() !== "") {
              folder = folderModel.parentFolder;
            }
          }

          Layout.rightMargin: 5
        }

        Item {
          Layout.fillWidth: true
          Layout.fillHeight: true
        }

        ToolButton {
          id: doneButton

          visible: fileSelector.selectFolder

          implicitHeight: 44

          text: qsTr("Select folder")

          onClicked: {
            ui.updateOutputPath(fileSelector.folder);
            ui.changePage("Encrypt.qml");
          }
        }

        Item {
          visible: fileSelector.selectFolder

          Layout.fillWidth: true
          Layout.fillHeight: true
        }
      }
    }
  }

  ListView {
    id: folderView
    model: folderModel
    delegate: fileRowDelegate
    footer: fileToolBar
    footerPositioning: ListView.OverlayFooter
    anchors.fill: parent
  }
}
