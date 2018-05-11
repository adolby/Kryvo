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

  QApplication app{argc, argv};
  app.setOrganizationName(QStringLiteral("The Kryvo Project"));
  app.setOrganizationDomain(QStringLiteral("kryvo.io"));
  app.setApplicationName(QStringLiteral("Kryvo"));

  Kryvo::Application kryvo;

  const int retval = app.exec();

  return retval;
}
