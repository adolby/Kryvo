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
  QApplication a{argc, argv};

  Kryvos kryvos{};

  auto retval = a.exec();

  return retval;
}
