#include "src/gui/DesktopMainWindow.hpp"
#include <QAction>
#include <QDropEvent>
#include <QMimeData>
#include <QUrl>

Kryvos::DesktopMainWindow::DesktopMainWindow(Settings* settings,
                                             QWidget* parent)
  : MainWindow{settings, parent} {
  this->settings = settings;

  messageFrame->appendText(tr("To begin, click the Add Files button or drag "
                              "and drop files. Next, enter a file path for the "
                              "output files. Enter a password. Finally, click "
                              "the Encrypt or Decrypt button."));

  // Adjust stretch of file list view
  contentLayout->setStretch(1, 200);

  // Adjust stretch of message box
  contentLayout->setStretch(2, 0);

  // Add files action
  auto addFilesAction = new QAction{this};
  addFilesAction->setShortcut(Qt::Key_O | Qt::CTRL);
  connect(addFilesAction, &QAction::triggered,
          this, &MainWindow::addFiles);
  this->addAction(addFilesAction);

  // Quit action
  auto quitAction = new QAction{this};
  quitAction->setShortcut(Qt::Key_Q | Qt::CTRL);
  connect(quitAction, &QAction::triggered,
          this, &QMainWindow::close);
  this->addAction(quitAction);

  this->move(this->settings->position());

  if (this->settings->maximized()) {
    // Move window, then maximize to ensure maximize occurs on correct screen
    this->setWindowState(this->windowState() | Qt::WindowMaximized);
  }
  else {
    this->resize(this->settings->size());
  }

  // Enable drag and drop
  this->setAcceptDrops(true);

  // Load stylesheet
  const QString styleSheet = loadStyleSheet(this->settings->styleSheetPath(),
                                            QStringLiteral("kryvos.qss"));

  if (!styleSheet.isEmpty()) {
    this->setStyleSheet(styleSheet);
  }
}

Kryvos::DesktopMainWindow::~DesktopMainWindow() {
}

void Kryvos::DesktopMainWindow::closeEvent(QCloseEvent* event) {
  this->settings->position(this->pos());

  if (this->isMaximized()) {
    this->settings->maximized(true);
  }
  else {
    this->settings->maximized(false);
    this->settings->size(this->size());
  }

  QMainWindow::closeEvent(event);
}

void Kryvos::DesktopMainWindow::dragEnterEvent(QDragEnterEvent* event) {
  // Show drag and drop as a move action
  event->setDropAction(Qt::MoveAction);

  if (event->mimeData()->hasUrls()) { // Accept drag and drops with files only
    event->accept();
  }
}

void Kryvos::DesktopMainWindow::dropEvent(QDropEvent* event) {
  // Check for the URL MIME type, which is a list of files
  if (event->mimeData()->hasUrls()) { // Extract the local path from the file(s)
    for (const QUrl& url : event->mimeData()->urls()) {
      fileListFrame->addFileToModel(url.toLocalFile());
    }
  }
}

QSize Kryvos::DesktopMainWindow::sizeHint() const {
  return QSize(800, 600);
}

QSize Kryvos::DesktopMainWindow::minimumSizeHint() const {
  return QSize(600, 420);
}
