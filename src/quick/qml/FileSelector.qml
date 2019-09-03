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
        id: backButton
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

        Label {
          text: fileSelector.folder
          elide: Text.ElideLeft
          font.pixelSize: 18
          width: parent.width
          anchors.centerIn: parent
        }
      }

      ToolButton {
        id: upButton
        implicitHeight: 44

        icon.source: "qrc:/images/up-128.png"
        icon.width: 30
        icon.height: 30

        Layout.rightMargin: 5

        onClicked: {
          if (folderModel.parentFolder.toString() !== "") {
            fileSelector.folder = folderModel.parentFolder;
          }
        }
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
    id: selectToolBar

    ToolBar {
      width: parent.width
      z: 2

      RowLayout {
        anchors.fill: parent

        Item {
          Layout.fillWidth: true
        }

        ToolButton {
          id: selectButton

          implicitHeight: 44

          text: qsTr("Select folder")

          onClicked: {
            ui.updateOutputPath(fileSelector.folder);
            ui.changePage("Encrypt.qml");
          }
        }

        Item {
          Layout.fillWidth: true
        }
      }
    }
  }

  ListView {
    id: folderView
    model: folderModel
    delegate: fileRowDelegate
    footer: fileSelector.selectFolder ? selectToolBar : null
    footerPositioning: ListView.OverlayFooter
    anchors.fill: parent
  }
}
