#include "src/cryptography/Crypto.hpp"
#include "src/cryptography/CryptoState.hpp"
#include "src/cryptography/BotanCrypto.hpp"
#include "src/cryptography/constants.h"
#include "src/utility/pimpl_impl.h"
#include <QFileInfo>
#include <QDir>
#include <QStringRef>
#include <QStringBuilder>
#include <string>
#include <memory>

#if defined(_MSC_VER)
#include "src/utility/make_unique.h"
#endif

QString outputFilePath(const QString& inputFilePath,
                       const QString& inputFileName,
                       const QString& outputPath)
{
  auto path = QString{inputFilePath % QStringLiteral(".") % kExtension};

  if (!outputPath.isEmpty())
  {
    path = QDir::cleanPath(outputPath) % QDir::separator() % inputFileName %
           QStringLiteral(".") % kExtension;
  }

  return path;
}

class Crypto::CryptoPrivate {
 public:
  /*!
   * \brief CryptoPrivate Constructs the Crypto private implementation
   */
  CryptoPrivate();

  /*!
   * \brief errorMessage Returns error message
   * \return String containing error message
   */
  QString errorMessage(const int index) const;

  std::unique_ptr<CryptoState> state;
  std::unique_ptr<BotanCrypto> botanCrypto;

  // The list of status messages that can be displayed to the user
  const QStringList messages;
};

Crypto::Crypto(QObject* parent)
  : QObject{parent}
{
  // Subscribe to provider's signals
  connect(m->botanCrypto.get(), &BotanCrypto::progress,
          this, &Crypto::progress);
  connect(m->botanCrypto.get(), &BotanCrypto::statusMessage,
          this, &Crypto::statusMessage);
  connect(m->botanCrypto.get(), &BotanCrypto::errorMessage,
          this, &Crypto::errorMessage);
}

Crypto::~Crypto() {}

void Crypto::encrypt(const QString& passphrase,
                     const QStringList& inputFilePaths,
                     const QString& outputPath,
                     const QString& cipher,
                     const std::size_t& inputKeySize,
                     const QString& modeOfOperation,
                     const bool compress,
                     const bool container)
{
  m->state->busy(true);
  emit busyStatus(m->state->isBusy());

  auto keySize = std::size_t{128};

  if (inputKeySize > 0)
  {
    keySize = inputKeySize;
  }

  auto algorithm = QStringLiteral("AES-128/GCM");

  if (QStringLiteral("AES") == cipher)
  {
    algorithm = cipher % QStringLiteral("-") % QString::number(keySize) %
                QStringLiteral("/") % modeOfOperation;
  }
  else
  {
    algorithm = cipher % QStringLiteral("/") % modeOfOperation;
  }

  auto outputFilePaths = QStringList{};

  for (const auto& inputFilePath : inputFilePaths)
  {
    const QFileInfo inputFileInfo{inputFilePath};
    const auto inFileName = inputFileInfo.fileName();
    const auto inFilePath = inputFileInfo.absoluteFilePath();
    const auto outFilePath = outputFilePath(inFilePath,inFileName,
                                            outputPath);

    outputFilePaths << outFilePath;

    try
    {
      m->botanCrypto->encryptFile(passphrase, inFilePath, outFilePath,
                                  algorithm, keySize, compress);

      if (m->state->isAborted() || m->state->isStopped(inFilePath))
      {
        emit errorMessage(m->messages[2], inFilePath);

        if (m->state->isAborted())
        {
          m->state->abort(false);
          break;
        }
      }
    }
    catch (const Botan::Stream_IO_Error& e)
    {
      emit errorMessage(m->messages[7], inFilePath);
    }
    catch (const std::exception& e)
    {
      const auto error = QString{e.what()};
      emit errorMessage(QStringLiteral("Error: ") % error, inFilePath);
    }
  } // End file loop

  m->state->reset();

  m->state->busy(false);
  emit busyStatus(m->state->isBusy());
}

void Crypto::decrypt(const QString& passphrase,
                     const QStringList& inputFilePaths,
                     const QString& outputPath)
{
  m->state->busy(true);
  emit busyStatus(m->state->isBusy());

  for (const auto& inputFilePath : inputFilePaths)
  {
    const QFileInfo inputFileInfo{inputFilePath};
    const auto inFileName = inputFileInfo.fileName();
    const auto inFilePath = inputFileInfo.absoluteFilePath();
    const auto outFilePath = outputFilePath(inFilePath,inFileName, outputPath);

    try
    {
      m->botanCrypto->decryptFile(passphrase, inFilePath, outFilePath);

      if (m->state->isAborted())
      { // Reset abort flag
        m->state->abort(false);
        emit errorMessage(m->messages[3], inFilePath);
        break;
      }

      if (m->state->isStopped(inFilePath))
      {
        emit errorMessage(m->messages[3], inFilePath);
      }
    }
    catch (const Botan::Decoding_Error& e)
    {
      emit errorMessage(m->messages[5], inFilePath);
    }
    catch (const Botan::Integrity_Failure& e)
    {
      emit errorMessage(m->messages[5], inFilePath);
    }
    catch (const Botan::Invalid_Argument& e)
    {
      emit errorMessage(m->messages[6], inFilePath);
    }
    catch (const std::invalid_argument& e)
    {
      emit errorMessage(m->messages[6], inFilePath);
    }
    catch (const std::runtime_error& e)
    {
      emit errorMessage(m->messages[4], inFilePath);
    }
    catch (const std::exception& e)
    {
      const auto error = QString{e.what()};

      if (QStringLiteral("zlib inflate error -3") == error)
      {
        emit errorMessage(m->messages[5], inFilePath);
      }
      else
      {
        emit errorMessage(QStringLiteral("Error: ") % error, inFilePath);
      }
    }
  }

  m->state->reset();

  m->state->busy(false);
  emit busyStatus(m->state->isBusy());
}

void Crypto::abort()
{
  if (m->state->isBusy())
  {
    m->state->abort(true);
  }
}

void Crypto::pause(bool pause)
{
  m->state->pause(pause);
}

void Crypto::stop(const QString& filePath)
{
  if (m->state->isBusy())
  {
    m->state->stop(filePath, true);
  }
}

Crypto::CryptoPrivate::CryptoPrivate()
  : state{std::make_unique<CryptoState>()},
    botanCrypto{std::make_unique<BotanCrypto>(state.get())}
{}
