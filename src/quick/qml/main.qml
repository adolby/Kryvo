import QtQuick 2.0
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

ApplicationWindow {
  property var currentPage: ui.currentPage

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
