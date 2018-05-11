#ifndef KRYVO_TESTS_FILEOPERATIONS_HPP_
#define KRYVO_TESTS_FILEOPERATIONS_HPP_

#include <QString>

namespace FileOperations {

template <class InputIterator1, class InputIterator2>
bool equal(InputIterator1 first1, InputIterator1 last1,
           InputIterator2 first2, InputIterator2 last2);

bool filesEqual(const QString& filePath1, const QString& filePath2);

}

#endif // KRYVO_TESTS_FILEOPERATIONS_HPP_
