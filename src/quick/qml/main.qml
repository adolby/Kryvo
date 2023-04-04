import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import QtQuick.Dialogs

ApplicationWindow {
  id: window

  width: 800
  height: 600

  visible: true

  Component.onCompleted: {
    ui.navigate("Encrypt.qml");
  }

  Connections {
    target: ui

    function onPushPage(page) {
      if (page.name) {
        if (page.properties) {
          pageLoader.push(page.name, page.properties);
        } else {
          pageLoader.push(page.name);
        }
      }
    }

    function onPopPage() {
      pageLoader.pop();
    }
  }

  StackView {
    id: pageLoader

    anchors.fill: parent

    pushEnter: Transition {
      PropertyAnimation {
        property: "opacity"
        from: 0
        to: 1
        duration: 80
      }
    }

    pushExit: Transition {
      PropertyAnimation {
        property: "opacity"
        from: 1
        to: 0
        duration: 80
      }
    }

    popEnter: Transition {
      PropertyAnimation {
        property: "opacity"
        from: 0
        to: 1
        duration: 80
      }
    }

    popExit: Transition {
      PropertyAnimation {
          property: "opacity"
          from: 1
          to: 0
          duration: 80
      }
    }
  }
}
