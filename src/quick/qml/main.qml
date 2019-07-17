import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import QtQuick.Controls.Material 2.2

ApplicationWindow {
  id: window

  property var currentPage: ui.currentPage

  width: 800
  height: 600

  visible: true

  onCurrentPageChanged: {
    if (currentPage.name) {
      if (currentPage.properties) {
        pageLoader.setSource(currentPage.name, currentPage.properties);
      } else {
        pageLoader.setSource(currentPage.name);
      }
    }
  }

  Component.onCompleted: {
    ui.navigate("Encrypt.qml");
  }

  Loader {
    id: pageLoader

    anchors.fill: parent
  }
}
