#include "Application.hpp"
#include <QApplication>

/*!
 * \brief main Main function
 * \param argc Number of command line parameters
 * \param argv Array of command line parameters
 * \return Integer exit code
 */
int main(int argc, char* argv[]) {
  QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

  QApplication app(argc, argv);
  QApplication::setOrganizationName(QStringLiteral("The Kryvo Project"));
  QApplication::setOrganizationDomain(QStringLiteral("kryvo.io"));
  QApplication::setApplicationName(QStringLiteral("Kryvo"));

  Kryvo::Application kryvo;

  const int retval = QApplication::exec();

  return retval;
}
