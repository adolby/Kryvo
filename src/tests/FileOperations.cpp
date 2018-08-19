#include "FileOperations.h"
#include <QFile>
#include <QtGlobal>

template <class InputIterator1, class InputIterator2>
bool FileOperations::equal(InputIterator1 first1, InputIterator1 last1,
                           InputIterator2 first2, InputIterator2 last2) {
  while ((first1 != last1) && (first2 != last2)) {
    if (*first1 != *first2) {
      return false;
    }

    ++first1;
    ++first2;
  }

  return (first1 == last1) && (first2 == last2);
}

bool FileOperations::filesEqual(const QString& filePath1,
                                const QString& filePath2) {
  bool equivalent = false;

  QFile file1(filePath1);
  QFile file2(filePath2);

  if (file1.exists() && file2.exists()) {
    file1.open(QFile::ReadOnly);
    uchar* fileMemory1 = file1.map(0, file1.size());

    file2.open(QFile::ReadOnly);
    uchar* fileMemory2 = file2.map(0, file2.size());

    equivalent = equal(fileMemory1, fileMemory1 + file1.size(),
                       fileMemory2, fileMemory2 + file2.size());
  }

  return equivalent;
}
