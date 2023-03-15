#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest.h>
#include <QMetaType>
#include <QCoreApplication>
#include <QtTest>

int main(int argc, char* argv[]) {
  QCoreApplication a(argc, argv);

  qRegisterMetaType<QFileInfo>("QFileInfo");

  QTEST_SET_MAIN_SOURCE_PATH

  doctest::Context context;

  context.applyCommandLine(argc, argv);

  int res = context.run();

  if (context.shouldExit()) { // important - query flags (and --exit) rely on the user doing this
    return res;
  }

  return res; // the result from doctest is propagated here as well
}
