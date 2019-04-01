import QtQuick.Controls 2.12

ApplicationWindow {
  visible: true

  menuBar: MenuBar {
      // ...
  }

  header: ToolBar {
      // ...
  }

  footer: ToolBar {
      // ...
  }

  StackView {
      anchors.fill: parent
  }
}
