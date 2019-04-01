#include "Application.hpp"
#include <QApplication>
#include <QCoreApplication>

/*!
 * \brief main Main function
 * \param argc Number of command line parameters
 * \param argv Array of command line parameters
 * \return Integer exit code
 */
int main(int argc, char* argv[]) {
  QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

  QApplication app(argc, argv);

  QCoreApplication::setOrganizationName(
    QStringLiteral("The Kryvo Project"));
  QCoreApplication::setOrganizationDomain(QStringLiteral("kryvo.io"));
  QCoreApplication::setApplicationName(QStringLiteral("Kryvo"));

  Kryvo::Application kryvo;

  const int retval = QApplication::exec();

  return retval;
}
