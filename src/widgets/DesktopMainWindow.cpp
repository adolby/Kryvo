#include "DesktopMainWindow.hpp"
#include <QAction>
#include <QDropEvent>
#include <QMimeData>
#include <QUrl>

namespace Kryvo {

DesktopMainWindow::DesktopMainWindow(Settings* s, QWidget* parent)
  : MainWindow(s, parent) {
  messageFrame->appendMessage(tr("To begin, click the Add Files button or drag "
                                 "and drop files. Next, enter a file path for "
                                 "the output files. Enter a password. Finally, "
                                 "click the Encrypt or Decrypt button."));

  // Adjust stretch of file list view
  contentLayout->setStretch(1, 200);

  // Adjust stretch of message box
  contentLayout->setStretch(2, 0);

  // Add files action
  auto addFilesAction = new QAction(this);
  addFilesAction->setShortcut(Qt::Key_O | Qt::CTRL);
  connect(addFilesAction, &QAction::triggered,
          this, &MainWindow::addFiles);
  this->addAction(addFilesAction);

  // Quit action
  auto quitAction = new QAction(this);
  quitAction->setShortcut(Qt::Key_Q | Qt::CTRL);
  connect(quitAction, &QAction::triggered,
          this, &QMainWindow::close);
  this->addAction(quitAction);

  // Enable drag and drop
  this->setAcceptDrops(true);

  connect(settings, &Settings::settingsImported,
          this, &DesktopMainWindow::settingsImported);
}

void DesktopMainWindow::settingsImported() {
  qInfo() << Q_FUNC_INFO;
  MainWindow::settingsImported();

  this->move(settings->position());

  if (settings->maximized()) {
    // Move window, then maximize to ensure maximize occurs on correct screen
    this->setWindowState(this->windowState() | Qt::WindowMaximized);
  } else {
    this->resize(settings->size());
  }

  // Load stylesheet
  const QString styleSheet = loadStyleSheet(settings->styleSheetPath(),
                                            QStringLiteral("kryvo.qss"));

  if (!styleSheet.isEmpty()) {
    this->setStyleSheet(styleSheet);
  }
}

void DesktopMainWindow::closeEvent(QCloseEvent* event) {
  emit requestUpdateClosePosition(this->pos());

  if (this->isMaximized()) {
    emit requestUpdateCloseMaximized(true);
  } else {
    emit requestUpdateCloseMaximized(false);
    emit requestUpdateCloseSize(this->size());
  }

  QMainWindow::closeEvent(event);
}

void DesktopMainWindow::dragEnterEvent(QDragEnterEvent* event) {
  // Show drag and drop as a move action
  event->setDropAction(Qt::MoveAction);

  if (event->mimeData()->hasUrls()) { // Accept drag and drops with files only
    event->accept();
  }
}

void DesktopMainWindow::dropEvent(QDropEvent* event) {
  // Check for the URL MIME type, which is a list of files
  if (event->mimeData()->hasUrls()) { // Extract the local path from the file(s)
    for (const QUrl& url : event->mimeData()->urls()) {
      fileListFrame->addFileToModel(QFileInfo(url.toLocalFile()));
    }
  }
}

QSize DesktopMainWindow::sizeHint() const {
  return QSize(800, 600);
}

QSize DesktopMainWindow::minimumSizeHint() const {
  return QSize(600, 420);
}

} // namespace Kryvo
