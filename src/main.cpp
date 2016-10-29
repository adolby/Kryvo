#include "src/Kryvos.hpp"
#include <QApplication>

/*!
 * \brief main Main function.
 * \param argc Number of command line parameters.
 * \param argv Array of command line parameters.
 * \return Integer exit code.
 */
int main(int argc, char* argv[])
{
  QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

  QApplication app{argc, argv};

  Kryvos kryvos{};

  auto retval = app.exec();

  return retval;
}
