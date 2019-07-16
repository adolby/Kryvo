#include "FileItem.hpp"

Kryvo::FileItem::FileItem(const QString& fileName)
  : mFileName(fileName) {
}

QString Kryvo::FileItem::fileName() const {
  return mFileName;
}

void Kryvo::FileItem::setFileName(const QString& file) {
  mFileName = file;
}

QString Kryvo::FileItem::task() const {
  return mTask;
}

void Kryvo::FileItem::setTask(const QString& task) {
  mTask = task;
}

int Kryvo::FileItem::progress() const {
  return mProgress;
}

void Kryvo::FileItem::setProgress(const int percent) {
  mProgress = percent;
}
