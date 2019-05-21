#include "Application.hpp"
#include <QGuiApplication>
#include <QCoreApplication>

/*!
 * \brief main Main function
 * \param argc Number of command line parameters
 * \param argv Array of command line parameters
 * \return Integer exit code
 */
int main(int argc, char* argv[]) {
  QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
  QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

  QGuiApplication app(argc, argv);

  QCoreApplication::setOrganizationName(
    QStringLiteral("The Kryvo Project"));
  QCoreApplication::setOrganizationDomain(QStringLiteral("kryvo.io"));
  QCoreApplication::setApplicationName(QStringLiteral("Kryvo"));

  Kryvo::Application kryvo;

  const int retval = QGuiApplication::exec();

  return retval;
}
