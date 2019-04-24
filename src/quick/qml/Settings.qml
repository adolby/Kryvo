import QtQuick 2.0
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

Page {
  background: Rectangle {
    color: "#2c3e50"
  }

  footer: ToolBar {
    background: Rectangle {
      implicitHeight: 44
      color: "#2c3e50"
    }

    RowLayout {
      anchors.fill: parent

      ToolButton {
        id: backButton

        text: qsTr("Back")
        icon.source: "qrc:/images/backIcon.png"
        icon.width: 12
        icon.height: 12
        display: AbstractButton.TextBesideIcon

        background: Rectangle {
          implicitHeight: 44
          radius: 3
          color: "#8e44ad"
        }

        onClicked: {
          ui.navigate("Encrypt.qml");
        }
      }
    }
  }
}
