#include "BotanCrypto.hpp"
#include "src/cryptography/constants.h"
#include "src/utility/pimpl_impl.h"
#include <QFileInfo>

const auto kKDFHash = std::string{"KDF2(Keccak-1600)"};
const auto kKeyLabel = std::string{"user secret"};
const auto kIVLabel = std::string{"initialization vector"};

class BotanCrypto::BotanCryptoPrivate {
 public:
  /*!
   * \brief CryptoPrivate Constructs the BotanCrypto private implementation
   */
  BotanCryptoPrivate();

  CryptoState* state;
};

BotanCrypto::BotanCryptoPrivate::BotanCryptoPrivate()
  : state{nullptr}
{}

BotanCrypto::BotanCrypto(CryptoState* state, QObject* parent)
  : QObject{parent}
{
  m->state = state;

  // Initialize Botan
  Botan::LibraryInitializer init{std::string{"thread_safe=true"}};
}

BotanCrypto::~BotanCrypto() {}

void BotanCrypto::encryptFile(const QString& passphrase,
                              const QString& inputFilePath,
                              const QString& outputFilePath,
                              const QString& algorithmName,
                              const std::size_t& keySize,
                              const bool compress)
{
  Q_ASSERT(m->state);

  const QFileInfo inputFileInfo{inputFilePath};

  if (!m->state->isAborted() && inputFileInfo.exists() &&
      inputFileInfo.isFile() && inputFileInfo.isReadable())
  {
    Botan::AutoSeeded_RNG rng{};

    // Define a size for the PBKDF salt vector
    const auto pbkdfSaltSize = static_cast<std::size_t>(256);
    Botan::secure_vector<Botan::byte> pbkdfSalt;
    pbkdfSalt.resize(pbkdfSaltSize);

    // Create random PBKDF salt
    rng.randomize(&pbkdfSalt[0], pbkdfSalt.size());

    // Set up the key derive functions
    const auto macSize = static_cast<std::size_t>(512);

    // PKCS5_PBKDF2 takes ownership of the new HMAC and the HMAC takes ownership
    // of the Keccak_1600 hash function object (both via unique_ptr)
    Botan::PKCS5_PBKDF2 pbkdf{new Botan::HMAC{new Botan::Keccak_1600{macSize}}};
    const auto PBKDF2_ITERATIONS = 15000;

    // Create the PBKDF key
    const auto pbkdfKeySize = static_cast<std::size_t>(256);
    Botan::secure_vector<Botan::byte> pbkdfKey =
      pbkdf.derive_key(pbkdfKeySize, passphrase.toStdString(), &pbkdfSalt[0],
                       pbkdfSalt.size(), PBKDF2_ITERATIONS).bits_of();

    // Create the key and IV
    std::unique_ptr<Botan::KDF> kdf{Botan::KDF::create(kKDFHash)};

    // Set up key salt size
    const auto keySaltSize = static_cast<std::size_t>(64);
    Botan::secure_vector<Botan::byte> keySalt;
    keySalt.resize(keySaltSize);
    rng.randomize(&keySalt[0], keySalt.size());

    // Key is constrained to sizes allowed by algorithm
    const auto keySizeInBytes = keySize / 8;
    const auto keyLabelVector =
      reinterpret_cast<const Botan::byte*>(kKeyLabel.data());
    Botan::SymmetricKey key{kdf->derive_key(keySizeInBytes,
                                            pbkdfKey.data(),
                                            pbkdfKey.size(),
                                            keySalt.data(),
                                            keySalt.size(),
                                            keyLabelVector,
                                            kKeyLabel.size())};

    // Set up IV salt size
    const auto ivSaltSize = static_cast<std::size_t>(64);
    Botan::secure_vector<Botan::byte> ivSalt;
    ivSalt.resize(ivSaltSize);
    rng.randomize(&ivSalt[0], ivSalt.size());

    const auto ivSize = static_cast<std::size_t>(256);
    const auto ivLabelVector =
      reinterpret_cast<const Botan::byte*>(kIVLabel.data());
    Botan::InitializationVector iv{kdf->derive_key(ivSize,
                                                   pbkdfKey.data(),
                                                   pbkdfKey.size(),
                                                   ivSalt.data(),
                                                   ivSalt.size(),
                                                   ivLabelVector,
                                                   kIVLabel.size())};

    std::ifstream in{inputFilePath.toStdString(), std::ios::binary};
    std::ofstream out{outputFilePath.toStdString(), std::ios::binary};

    const auto algorithmNameStd = algorithmName.toStdString();

    auto headerText = tr("-------- ENCRYPTED FILE --------");

    auto compressedText = tr("Not compressed");

    if (compress)
    {
      compressedText = tr("Compressed");
    }

    out << headerText.toStdString() << std::endl;
    out << algorithmNameStd << std::endl;
    out << keySize << std::endl;
    out << compressedText.toStdString() << std::endl;
    out << Botan::base64_encode(&pbkdfSalt[0], pbkdfSalt.size()) << std::endl;
    out << Botan::base64_encode(&keySalt[0], keySalt.size()) << std::endl;
    out << Botan::base64_encode(&ivSalt[0], ivSalt.size()) << std::endl;

    Botan::Pipe pipe{};

    if (compress)
    {
      pipe.append(new Botan::Compression_Filter{std::string{"zlib"},
                                                static_cast<std::size_t>(9)});
    }

    pipe.append(Botan::get_cipher(algorithmNameStd,
                                  key,
                                  iv,
                                  Botan::ENCRYPTION));

    pipe.append(new Botan::DataSink_Stream{out});

    executeCipher(inputFilePath, pipe, in, out);

    if (!m->state->isAborted() && !m->state->isStopped(inputFilePath))
    {
      // Progress: finished
      emit progress(inputFilePath, 100);

      // Encryption success message
      emit statusMessage(messages[0].arg(inputFilePath));
    }
  }
}

void BotanCrypto::decryptFile(const QString& passphrase,
                              const QString& inputFilePath,
                              const QString& outputPath)
{
  Q_ASSERT(m->state);

  const QFileInfo inputFileInfo{inputFilePath};

  if (!m->state->isAborted() && inputFileInfo.exists() &&
      inputFileInfo.isFile() && inputFileInfo.isReadable())
  {
    std::ifstream in{inputFilePath.toStdString(), std::ios::binary};

    // Read metadata from file
    std::string headerStringStd, algorithmNameStd, keySizeString,
                compressStringStd, containerStringStd;
    std::string pbkdfSaltString, keySaltString, ivSaltString;

    std::getline(in, headerStringStd);
    std::getline(in, algorithmNameStd);
    std::getline(in, keySizeString);
    std::getline(in, compressStringStd);

    std::getline(in, pbkdfSaltString);
    std::getline(in, keySaltString);
    std::getline(in, ivSaltString);

    auto headerString = QString{headerStringStd.c_str()};
    auto containerString = QString{containerStringStd.c_str()};
    auto compressString = QString{compressStringStd.c_str()};

    if (headerString != tr("-------- ENCRYPTED FILE --------"))
    {
      emit errorMessage(messages[6].arg(inputFilePath));
    }

    // TODO: Extract files from container and process them
    // extract(passphrase, inputFilePath, outputPath);

    // Set up the key derive functions
    const auto macSize = static_cast<std::size_t>(512);

    // PKCS5_PBKDF2 takes ownership of the new HMAC and the HMAC takes ownership
    // of the Keccak_1600 hash function object (via unique_ptr)
    Botan::PKCS5_PBKDF2 pbkdf{new Botan::HMAC{new Botan::Keccak_1600{macSize}}};
    const auto PBKDF2_ITERATIONS = static_cast<std::size_t>(15000);

    // Create the PBKDF key
    Botan::secure_vector<Botan::byte> pbkdfSalt =
      Botan::base64_decode(pbkdfSaltString);
    const auto pbkdfKeySize = static_cast<std::size_t>(256);
    Botan::secure_vector<Botan::byte> pbkdfKey =
      pbkdf.derive_key(pbkdfKeySize, passphrase.toStdString(), &pbkdfSalt[0],
                       pbkdfSalt.size(), PBKDF2_ITERATIONS).bits_of();

    // Create the key and IV
    std::unique_ptr<Botan::KDF> kdf{Botan::KDF::create(kKDFHash)};

    // Key salt
    Botan::secure_vector<Botan::byte> keySalt =
        Botan::base64_decode(keySaltString);
    const auto keySize =
      static_cast<std::size_t>(QString{keySizeString.c_str()}.toInt());
    const auto keySizeInBytes = keySize / 8;
    const auto keyLabelVector =
      reinterpret_cast<const Botan::byte*>(kKeyLabel.data());
    Botan::SymmetricKey key{kdf->derive_key(keySizeInBytes,
                                            pbkdfKey.data(),
                                            pbkdfKey.size(),
                                            keySalt.data(),
                                            keySalt.size(),
                                            keyLabelVector,
                                            kKeyLabel.size())};

    Botan::secure_vector<Botan::byte> ivSalt =
      Botan::base64_decode(ivSaltString);
    const auto ivSize = static_cast<std::size_t>(256);
    const auto ivLabelVector =
      reinterpret_cast<const Botan::byte*>(kIVLabel.data());
    Botan::InitializationVector iv{kdf->derive_key(ivSize,
                                                   pbkdfKey.data(),
                                                   pbkdfKey.size(),
                                                   ivSalt.data(),
                                                   ivSalt.size(),
                                                   ivLabelVector,
                                                   kIVLabel.size())};

    // Remove the .enc extension if it's in the file path
    const auto outputFilePath = removeExtension(inputFilePath, kExtension);

    // Create a unique file name for the file in this directory
    const auto uniqueOutputFilePath = uniqueFilePath(outputFilePath);

    std::ofstream out{uniqueOutputFilePath.toStdString(), std::ios::binary};

    Botan::Pipe pipe{};

    pipe.append(Botan::get_cipher(algorithmNameStd,
                                  key,
                                  iv,
                                  Botan::DECRYPTION));

    if (tr("Compressed") == compressString)
    {
      pipe.append(new Botan::Decompression_Filter{std::string{"zlib"},
                                                  static_cast<std::size_t>(9)});
    }

    pipe.append(new Botan::DataSink_Stream{out});

    executeCipher(inputFilePath, pipe, in, out);

    if (!m->state->isAborted() && !m->state->isStopped(inputFilePath))
    {
      // Progress: finished
      emit progress(inputFilePath, 100);

      // Decryption success message
      emit statusMessage(messages[1].arg(inputFilePath));
    }
  }
}

void BotanCrypto::executeCipher(const QString& inputFilePath,
                                Botan::Pipe& pipe,
                                std::ifstream& in,
                                std::ofstream& out)
{
  Q_ASSERT(m->state);

  // Define a size for the buffer vector
  const auto bufferSize = static_cast<std::size_t>(4096);
  Botan::secure_vector<Botan::byte> buffer;
  buffer.resize(bufferSize);

  // Get file size for percent progress calculation
  const QFileInfo file{inputFilePath};
  const qint64 size = file.size();
  std::size_t fileIndex = 0;
  qint64 percent = -1;

  pipe.start_msg();

  while (in.good() && !m->state->isAborted() &&
         !m->state->isStopped(inputFilePath))
  {
    if (!m->state->isPaused())
    {
      in.read(reinterpret_cast<char*>(&buffer[0]), buffer.size());
      const auto remainingSize = in.gcount();
      pipe.write(&buffer[0], remainingSize);

      // Calculate progress in percent
      fileIndex += remainingSize;
      const auto nextFraction = static_cast<double>(fileIndex) /
                                static_cast<double>(size);
      const qint64 nextPercent = static_cast<qint64>(nextFraction * 100);

      if (nextPercent > percent && nextPercent < 100)
      {
        percent = nextPercent;
        emit progress(inputFilePath, percent);
      }

      if (in.eof())
      {
        pipe.end_msg();
      }

      while (pipe.remaining() > 0)
      {
        const auto buffered = pipe.read(&buffer[0], buffer.size());
        out.write(reinterpret_cast<const char*>(&buffer[0]), buffered);
      }
    }
  }

  if (in.bad() || (in.fail() && !in.eof()))
  {
    emit errorMessage(messages[4], inputFilePath);
  }

  out.flush();
}
