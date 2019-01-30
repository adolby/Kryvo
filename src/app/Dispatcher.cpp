#include "Dispatcher.hpp"
#include "DispatcherState.hpp"
#include "archive/Archiver.hpp"
#include "cryptography/Crypto.hpp"
#include "Constants.hpp"
#include "utility/Thread.hpp"
#include <QCoreApplication>

#include <QDebug>

class Kryvo::DispatcherPrivate {
  Q_DISABLE_COPY(DispatcherPrivate)
  Q_DECLARE_PUBLIC(Dispatcher)

 public:
  explicit DispatcherPrivate(Dispatcher* q);

  void dispatch();

  void processPipeline(std::size_t id);

  void abortPipeline(std::size_t id);

  void encrypt(const QString& passphrase,
               const QStringList& inputFilePaths,
               const QString& outputPath,
               const QString& cipher,
               std::size_t inputKeySize,
               const QString& modeOfOperation,
               bool compress,
               bool removeIntermediateFiles);

  void decrypt(const QString& passphrase,
               const QStringList& inputFilePaths,
               const QString& outputPath,
               bool removeIntermediateFiles);

  Dispatcher* const q_ptr{nullptr};

  DispatcherState state;

  Archiver archiver;
  Thread archiverThread;
  Crypto cryptographer;
  Thread cryptographerThread;

  std::vector<Pipeline> pipelines;
};

Kryvo::DispatcherPrivate::DispatcherPrivate(Dispatcher* dispatcher)
  : q_ptr(dispatcher), archiver(&state), cryptographer(&state) {
  archiver.moveToThread(&archiverThread);
  cryptographer.moveToThread(&cryptographerThread);

  QObject::connect(&archiver, &Archiver::fileProgress,
                   q_ptr, &Dispatcher::updateFileProgress);

  QObject::connect(&archiver, &Archiver::statusMessage,
                   q_ptr, &Dispatcher::statusMessage);

  QObject::connect(&archiver, &Archiver::errorMessage,
                   q_ptr, &Dispatcher::errorMessage);

  QObject::connect(q_ptr, &Dispatcher::compressFile,
                   &archiver, &Archiver::compress);

  QObject::connect(q_ptr, &Dispatcher::decompressFile,
                   &archiver, &Archiver::decompress);

  QObject::connect(&archiver, &Archiver::fileCompleted,
                   q_ptr, &Dispatcher::processPipeline);

  QObject::connect(&archiver, &Archiver::fileFailed,
                   q_ptr, &Dispatcher::abortPipeline);

  QObject::connect(&cryptographer, &Crypto::fileProgress,
                   q_ptr, &Dispatcher::updateFileProgress);

  QObject::connect(&cryptographer, &Crypto::statusMessage,
                   q_ptr, &Dispatcher::statusMessage);

  QObject::connect(&cryptographer, &Crypto::errorMessage,
                   q_ptr, &Dispatcher::errorMessage);

  QObject::connect(q_ptr, &Dispatcher::encryptFile,
                   &cryptographer, &Crypto::encrypt);

  QObject::connect(q_ptr, &Dispatcher::decryptFile,
                   &cryptographer, &Crypto::decrypt);

  QObject::connect(&cryptographer, &Crypto::fileCompleted,
                   q_ptr, &Dispatcher::processPipeline);

  QObject::connect(&cryptographer, &Crypto::fileFailed,
                   q_ptr, &Dispatcher::abortPipeline);

  archiverThread.start();
  cryptographerThread.start();
}

void Kryvo::DispatcherPrivate::dispatch() {
  Q_Q(Dispatcher);

  bool finished = true;

  for (std::size_t i = 0; i < pipelines.size(); ++i) {
    const Pipeline& pipeline = pipelines.at(i);

    if (pipeline.stage < pipeline.stages.size()) {
      finished = false;
      processPipeline(i);
      break;
    }
  }

  if (finished) {
    state.busy(false);
    emit q->busyStatus(state.isBusy());
  }
}

void Kryvo::DispatcherPrivate::processPipeline(const std::size_t id) {
  if (id >= pipelines.size()) {
    return;
  }

  Pipeline pipeline = pipelines.at(id);

  if (pipeline.stage >= pipeline.stages.size()) {
    dispatch();
    return;
  }

  const std::function<void(std::size_t)>& func =
    pipeline.stages.at(pipeline.stage);

  pipeline.stage = pipeline.stage + 1;

  pipelines[id] = pipeline;

  func(id);
}

void Kryvo::DispatcherPrivate::abortPipeline(const std::size_t id) {
  if (id >= pipelines.size()) {
    return;
  }

  Pipeline pipeline = pipelines.at(id);

  pipeline.stage = pipeline.stages.size();

  pipelines[id] = pipeline;

  dispatch();
}

void Kryvo::DispatcherPrivate::encrypt(const QString& passphrase,
                                       const QStringList& inputFilePaths,
                                       const QString& outputPath,
                                       const QString& cipher,
                                       const std::size_t inputKeySize,
                                       const QString& modeOfOperation,
                                       const bool compress,
                                       const bool removeIntermediateFiles) {
  Q_Q(Dispatcher);

  state.busy(true);
  emit q->busyStatus(state.isBusy());

  pipelines.clear();

  std::size_t id = 0;

  for (const QString& inputFilePath : inputFilePaths) {
    const std::size_t keySize = [&inputKeySize]() {
      std::size_t size = 128;

      if (inputKeySize > 0) {
        size = inputKeySize;
      }

      return size;
    }();

    const QDir outputDir(outputPath);

    // Create output path if it doesn't exist
    if (!outputDir.exists()) {
      outputDir.mkpath(outputPath);
    }

    const QFileInfo inputFileInfo(inputFilePath);
    const QString& inFilePath = inputFileInfo.absoluteFilePath();

    const QString& outPath = outputDir.exists() ?
                             outputDir.absolutePath() :
                             inputFileInfo.absolutePath();

    Pipeline pipeline;

    pipeline.inputFilePath = inFilePath;

    id = id + 1;

    if (compress) {
      const QString& compressedFilePath =
        QString(outPath % QStringLiteral("/") % inputFileInfo.fileName() %
                Kryvo::Constants::kDot %
                Kryvo::Constants::kCompressedFileExtension);

      auto compressFunction =
        [this, q, inFilePath, compressedFilePath](std::size_t id) {
          emit q->compressFile(id, inFilePath, compressedFilePath);
        };

      pipeline.stages.push_back(compressFunction);

      const QString& encryptedFilePath =
        QString(compressedFilePath % Kryvo::Constants::kDot %
                Kryvo::Constants::kEncryptedFileExtension);

      auto encryptFunction =
        [this, q, passphrase, compressedFilePath, encryptedFilePath, cipher,
         keySize, modeOfOperation, compress](std::size_t id) {
          emit q->encryptFile(id, passphrase, compressedFilePath,
                              encryptedFilePath, cipher, keySize,
                              modeOfOperation, compress);
        };

      pipeline.stages.push_back(encryptFunction);

      if (removeIntermediateFiles) {
        auto removeIntermediateFilesFunction =
          [this, compressedFilePath](std::size_t id) {
            QFile::remove(compressedFilePath);
          };

        pipeline.stages.push_back(removeIntermediateFilesFunction);
      }
    } else {
      const QString& encryptedFilePath =
        QString(outPath % QStringLiteral("/") % inputFileInfo.fileName() %
                Kryvo::Constants::kDot %
                Kryvo::Constants::kEncryptedFileExtension);

      auto encryptFunction =
        [this, q, passphrase, inFilePath, encryptedFilePath, cipher, keySize,
         modeOfOperation, compress](std::size_t id) {
          emit q->encryptFile(id, passphrase, inFilePath, encryptedFilePath,
                              cipher, keySize, modeOfOperation, compress);
        };

      pipeline.stages.push_back(encryptFunction);
    }

    pipelines.push_back(pipeline);
  }

  state.init(id);

  dispatch();
}

void Kryvo::DispatcherPrivate::decrypt(const QString& passphrase,
                                       const QStringList& inputFilePaths,
                                       const QString& outputPath,
                                       const bool removeIntermediateFiles) {
  Q_Q(Dispatcher);

  state.busy(true);
  emit q->busyStatus(state.isBusy());

  pipelines.clear();

  std::size_t id = 0;

  for (const QString& inputFilePath : inputFilePaths) {
    const QFileInfo inputFileInfo(inputFilePath);
    const QString& inFilePath = inputFileInfo.absoluteFilePath();

    QFile inFile(inFilePath);

    const bool inFileOpen = inFile.open(QIODevice::ReadOnly);

    if (!inFileOpen) {
      emit q->errorMessage(Constants::messages[5], inFilePath);
    }

    // Read metadata from file

    // Read line but skip \n
    auto readLine = [](QFile* file) {
      if (file) {
        QByteArray line = file->readLine();
        return line.replace(QByteArrayLiteral("\n"), QByteArrayLiteral(""));
      }

      return QByteArray();
    };

    const QByteArray& headerString = readLine(&inFile);

    if (headerString != QByteArrayLiteral("-------- ENCRYPTED FILE --------")) {
      emit q->errorMessage(Constants::messages[7], inFilePath);
    }

    const QString& algorithmNameString = readLine(&inFile);
    const QString& keySizeString = readLine(&inFile);
    const QString& compressString = readLine(&inFile);

    const QString& pbkdfSaltString = readLine(&inFile);
    const QString& keySaltString = readLine(&inFile);
    const QString& ivSaltString = readLine(&inFile);

    const QByteArray& footerString = readLine(&inFile);

    if (footerString !=
        QByteArrayLiteral("---------------------------------")) {
      emit q->errorMessage(Constants::messages[7], inFilePath);
    }

    const QDir outputDir(outputPath);

    // Create output path if it doesn't exist
    if (!outputDir.exists()) {
      outputDir.mkpath(outputPath);
    }

    const QString& outPath = outputDir.exists() ?
                             outputDir.absolutePath() :
                             inputFileInfo.absolutePath();

    const QString& outputFilePath = QString(outPath % QStringLiteral("/") %
                                            inputFileInfo.fileName());

    // Remove the .enc extensions if at the end of the file path
    const QString& decryptedFilePath =
      Constants::removeExtension(outputFilePath,
                                 Constants::kEncryptedFileExtension);

    // Create a unique file name for the file in this directory
    const QString& uniqueDecryptedFilePath =
      Constants::uniqueFilePath(decryptedFilePath);

    Pipeline pipeline;

    pipeline.inputFilePath = inFilePath;

    id = id + 1;

    if (QByteArrayLiteral("Gzip Compressed") == compressString) {
      // Remove the gz extension if at the end of the file path
      const QString& decompressedFilePath =
        Constants::removeExtension(outputFilePath,
                                   Constants::kCompressedFileExtension);

      // Create a unique file name for the file in this directory
      const QString& uniqueDecompressedFilePath =
        Constants::uniqueFilePath(decompressedFilePath);

      auto decompress =
        [this, q, inFilePath, uniqueDecompressedFilePath](std::size_t id) {
          emit q->decompressFile(id, inFilePath, uniqueDecompressedFilePath);
        };

      pipeline.stages.push_back(decompress);

      auto decrypt =
        [this, q, passphrase, uniqueDecompressedFilePath,
         uniqueDecryptedFilePath, algorithmNameString, keySizeString,
         pbkdfSaltString, keySaltString, ivSaltString](std::size_t id) {
          emit q->decryptFile(id, passphrase, uniqueDecompressedFilePath,
                              uniqueDecryptedFilePath, algorithmNameString,
                              keySizeString, pbkdfSaltString, keySaltString,
                              ivSaltString);
        };

      pipeline.stages.push_back(decrypt);

      if (removeIntermediateFiles) {
        auto removeIntermediateFilesFunction =
          [this, uniqueDecompressedFilePath](std::size_t id) {
            QFile::remove(uniqueDecompressedFilePath);
          };

        pipeline.stages.push_back(removeIntermediateFilesFunction);
      }
    } else {
        auto decrypt =
          [this, q, passphrase, inFilePath, uniqueDecryptedFilePath,
           algorithmNameString, keySizeString, pbkdfSaltString, keySaltString,
           ivSaltString](std::size_t id) {
            emit q->decryptFile(id, passphrase, inFilePath,
                                uniqueDecryptedFilePath, algorithmNameString,
                                keySizeString, pbkdfSaltString, keySaltString,
                                ivSaltString);
          };

        pipeline.stages.push_back(decrypt);
    }

    pipelines[id] = pipeline;
  }

  state.init(id);

  dispatch();
}

Kryvo::Dispatcher::Dispatcher(QObject* parent)
  : QObject(parent), d_ptr(std::make_unique<DispatcherPrivate>(this)) {
}

Kryvo::Dispatcher::~Dispatcher() = default;

void Kryvo::Dispatcher::encrypt(const QString& passphrase,
                                const QStringList& inputFilePaths,
                                const QString& outputPath,
                                const QString& cipher,
                                const std::size_t inputKeySize,
                                const QString& modeOfOperation,
                                const bool compress,
                                const bool removeIntermediateFiles) {
  Q_D(Dispatcher);

  d->encrypt(passphrase, inputFilePaths, outputPath, cipher, inputKeySize,
             modeOfOperation, compress, removeIntermediateFiles);
}

void Kryvo::Dispatcher::decrypt(const QString& passphrase,
                                const QStringList& inputFilePaths,
                                const QString& outputPath,
                                const bool removeIntermediateFiles) {
  Q_D(Dispatcher);

  d->decrypt(passphrase, inputFilePaths, outputPath, removeIntermediateFiles);
}

void Kryvo::Dispatcher::abort() {
  Q_D(Dispatcher);

  if (d->state.isBusy()) {
    d->state.abort(true);
  }
}

void Kryvo::Dispatcher::pause(const bool pause) {
  Q_D(Dispatcher);

  d->state.pause(pause);
}

void Kryvo::Dispatcher::stop(const QString& filePath) {
  Q_D(Dispatcher);

  if (d->state.isBusy()) {
    std::size_t id = 0;
    bool found = false;

    for (std::size_t i = 0; i < d->pipelines.size(); ++i) {
      const Pipeline& pipeline = d->pipelines.at(i);

      if (pipeline.inputFilePath == filePath) {
        id = i;
        found = true;
        break;
      }
    }

    if (found) {
      d->state.stop(id, true);
    }
  }
}

void Kryvo::Dispatcher::processPipeline(const std::size_t id) {
  Q_D(Dispatcher);

  d->processPipeline(id);
}

void Kryvo::Dispatcher::abortPipeline(const std::size_t id) {
  Q_D(Dispatcher);

  d->abortPipeline(id);
}

void Kryvo::Dispatcher::updateFileProgress(const std::size_t id,
                                           const QString& task,
                                           const qint64 percentProgress) {
  Q_D(Dispatcher);

  if (id >= d->pipelines.size()) {
    return;
  }

  const Pipeline& pipeline = d->pipelines.at(id);

  emit fileProgress(pipeline.inputFilePath, task, percentProgress);
}
