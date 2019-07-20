#define CATCH_CONFIG_RUNNER
#include "catch.hpp"

#include <QMetaType>
#include <QCoreApplication>
#include <QtTest>

int main(int argc, char* argv[]) {
  QCoreApplication a(argc, argv);

  qRegisterMetaType<QFileInfo>("QFileInfo");

  QTEST_SET_MAIN_SOURCE_PATH
  const int result = Catch::Session().run(argc, argv);

  return (result < 0xFF ? result : 0xFF);
}
