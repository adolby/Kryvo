/*!
 * \brief main Main function
 * \param argc Number of command line parameters
 * \param argv Array of command line parameters
 * \return Integer exit code
 */
int main(int argc, char* argv[]) {
  QApplication app{argc, argv};
  app.setOrganizationName("The Kryvos Project");
  app.setOrganizationDomain("andrewdolby.com");
  app.setApplicationName("Kryvos");

  Kryvos::Application kryvos{};

  const auto retval = app.exec();

  return retval;
}
