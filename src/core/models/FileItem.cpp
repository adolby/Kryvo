#include "models/FileItem.hpp"

namespace Kryvo {

FileItem::FileItem(const QString& fileName)
  : fileName_(fileName) {
}

QString FileItem::fileName() const {
  return fileName_;
}

void FileItem::setFileName(const QString& file) {
  fileName_ = file;
}

QString FileItem::task() const {
  return task_;
}

void FileItem::setTask(const QString& task) {
  task_ = task;
}

int FileItem::progress() const {
  return progress_;
}

void FileItem::setProgress(const int percent) {
  progress_ = percent;
}

} // namespace Kryvo
