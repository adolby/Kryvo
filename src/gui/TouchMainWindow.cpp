#include "src/gui/TouchMainWindow.hpp"
#include <QApplication>

Kryvos::TouchMainWindow::TouchMainWindow(Settings* settings, QWidget* parent)
  : MainWindow{settings, parent} {
  messageFrame->appendText(tr("To begin, tap the Add Files button. Enter a "
                              "password. Finally, tap the Encrypt or Decrypt "
                              "button."));

  const QSize headerIconSize{50, 50};
  const QSize controlIconSize{50, 50};
  headerFrame->setIconSize(headerIconSize);
  controlButtonFrame->setIconSize(controlIconSize);

  // Adjust stretch of file list view
  contentLayout->setStretch(1, 3);
  // Adjust stretch of message box
  contentLayout->setStretch(2, 1);

  // Load touch-optimized stylesheet
  const auto& styleSheet = loadStyleSheet(settings->styleSheetPath(),
                                          QStringLiteral("kryvosTouch.qss"));

  if (!styleSheet.isEmpty()) {
    this->setStyleSheet(styleSheet);
  }
}

Kryvos::TouchMainWindow::~TouchMainWindow() {
}
