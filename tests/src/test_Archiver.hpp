#ifndef KRYVOS_TESTS_TEST_ARCHIVER_HPP_
#define KRYVOS_TESTS_TEST_ARCHIVER_HPP_

#include <QObject>

class TestArchiver : public QObject {
  Q_OBJECT

public:
  void testComparatorSameFile();
  void testComparatorDifferentFile();
};

#endif // KRYVOS_TESTS_TEST_ARCHIVER_HPP_
