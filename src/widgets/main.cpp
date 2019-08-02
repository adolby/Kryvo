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
  QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

  Kryvo::Application app(argc, argv);

  QCoreApplication::setOrganizationName(
    QStringLiteral("The Kryvo Project"));
  QCoreApplication::setOrganizationDomain(QStringLiteral("kryvo.io"));
  QCoreApplication::setApplicationName(QStringLiteral("Kryvo"));

  const int retval = Kryvo::Application::exec();

  return retval;
}
