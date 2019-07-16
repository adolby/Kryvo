#ifndef KRYVO_FILEITEM_HPP_
#define KRYVO_FILEITEM_HPP_

#include <QString>

namespace Kryvo {

class FileItem {
 public:
  FileItem(const QString& fileName);

  QString fileName() const;
  void setFileName(const QString& file);
  QString task() const;
  void setTask(const QString& task);
  int progress() const;
  void setProgress(int percent);

 private:
  QString mFileName;
  QString mTask;
  int mProgress = -1;
};

inline bool operator== (const FileItem& item1, const FileItem& item2) {
  return item1.fileName() == item2.fileName();
}

} // namespace Kryvo

#endif // KRYVO_FILEITEM_HPP_
