#include "models/FileItem.hpp"

Kryvo::FileItem::FileItem(const QString& fileName)
  : fileName_(fileName) {
}

QString Kryvo::FileItem::fileName() const {
  return fileName_;
}

void Kryvo::FileItem::setFileName(const QString& file) {
  fileName_ = file;
}

QString Kryvo::FileItem::task() const {
  return task_;
}

void Kryvo::FileItem::setTask(const QString& task) {
  task_ = task;
}

int Kryvo::FileItem::progress() const {
  return progress_;
}

void Kryvo::FileItem::setProgress(const int percent) {
  progress_ = percent;
}
