#include "Dispatcher.hpp"
#include "DispatcherState.hpp"
#include "PluginLoader.hpp"
#include "archive/Archiver.hpp"
#include "cryptography/Crypto.hpp"
#include "Constants.hpp"
#include "utility/Thread.hpp"
#include <QTimer>
#include <QStringBuilder>

class Kryvo::DispatcherPrivate {
  Q_DISABLE_COPY(DispatcherPrivate)
  Q_DECLARE_PUBLIC(Dispatcher)

 public:
  explicit DispatcherPrivate(Dispatcher* disp);

  void init();

  bool isComplete() const;

  void dispatch();

  void processPipeline(std::size_t id);

  void abortPipeline(std::size_t id);

  void encrypt(const QString& passphrase,
               const std::vector<QFileInfo>& inputFilePaths,
               const QDir& outputPath,
               const QString& cipher,
               std::size_t inputKeySize,
               const QString& modeOfOperation,
               bool compress,
               bool removeIntermediateFiles);

  void decrypt(const QString& passphrase,
               const std::vector<QFileInfo>& inputFilePaths,
               const QDir& outputPath,
               bool removeIntermediateFiles);

  Dispatcher* const q_ptr{nullptr};

  DispatcherState state;

  PluginLoader pluginLoader;

  Archiver archiver;
  Thread archiverThread;
  Crypto cryptographer;
  Thread cryptographerThread;

  std::vector<Pipeline> pipelines;
};

Kryvo::DispatcherPrivate::DispatcherPrivate(Dispatcher* disp)
  : q_ptr(disp), archiver(&state), cryptographer(&state) {
  archiver.moveToThread(&archiverThread);
  cryptographer.moveToThread(&cryptographerThread);

  QObject::connect(q_ptr, &Dispatcher::fileCompleted,
                   q_ptr, &Dispatcher::processPipeline,
                   Qt::QueuedConnection);

  QObject::connect(&pluginLoader, &PluginLoader::cryptoProviderChanged,
                   &cryptographer, &Crypto::updateProvider);

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
                   q_ptr, &Dispatcher::fileCompleted);

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
                   q_ptr, &Dispatcher::fileCompleted);

  QObject::connect(&cryptographer, &Crypto::fileFailed,
                   q_ptr, &Dispatcher::abortPipeline);

  archiverThread.start();
  cryptographerThread.start();

  QTimer::singleShot(0, q_ptr,
                     [this]() {
                       init();
                     });
}

void Kryvo::DispatcherPrivate::init() {
  pluginLoader.loadPlugins();
}

bool Kryvo::DispatcherPrivate::isComplete() const {
  Q_Q(const Dispatcher);

  bool finished = true;

  for (const Pipeline& pipeline : pipelines) {
    if (pipeline.stage < pipeline.stages.size()) {
      finished = false;
      break;
    }
  }

  return finished;
}

void Kryvo::DispatcherPrivate::dispatch() {
  Q_Q(Dispatcher);

  for (std::size_t i = 0; i < pipelines.size(); ++i) {
    QTimer::singleShot(0, q, [this, i]() {
                               processPipeline(i);
                             });
  }
}

void Kryvo::DispatcherPrivate::processPipeline(const std::size_t id) {
  Q_Q(Dispatcher);

  if (id >= pipelines.size()) {
    const bool complete = isComplete();

    if (complete) {
      state.busy(false);
      emit q->busyStatus(state.isBusy());
    }

    return;
  }

  Pipeline pipeline = pipelines.at(id);

  const bool complete = isComplete();

  if (complete) {
    state.busy(false);
    emit q->busyStatus(state.isBusy());

    return;
  }

  if (pipeline.stage >= pipeline.stages.size()) {
    return;
  }

  const std::function<void(std::size_t)>& func =
    pipeline.stages.at(pipeline.stage);

  pipeline.stage = pipeline.stage + 1;

  pipelines[id] = pipeline;

  func(id);
}

void Kryvo::DispatcherPrivate::abortPipeline(const std::size_t id) {
  Q_Q(Dispatcher);

  if (id >= pipelines.size()) {
    const bool complete = isComplete();

    if (complete) {
      state.busy(false);
      emit q->busyStatus(state.isBusy());
    }

    return;
  }

  Pipeline pipeline = pipelines.at(id);

  pipeline.stage = pipeline.stages.size();

  pipelines[id] = pipeline;

  const bool complete = isComplete();

  if (complete) {
    state.busy(false);
    emit q->busyStatus(state.isBusy());
  }
}

void Kryvo::DispatcherPrivate::encrypt(const QString& passphrase,
                                       const std::vector<QFileInfo>& inputFiles,
                                       const QDir& outputPath,
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

  for (const QFileInfo& inputFile : inputFiles) {
    const std::size_t keySize = [&inputKeySize]() {
      std::size_t size = 128;

      if (inputKeySize > 0) {
        size = inputKeySize;
      }

      return size;
    }();

    // Create output path if it doesn't exist
    if (!outputPath.exists()) {
      outputPath.mkpath(outputPath.absolutePath());
    }

    const QString& inFilePath = inputFile.absoluteFilePath();

    const QString& outPath = outputPath.exists() ?
                             outputPath.absolutePath() :
                             inputFile.absolutePath();

    Pipeline pipeline;

    pipeline.inputFilePath = inFilePath;

    id = id + 1;

    if (compress) {
      const QString& compressedFilePath =
        QString(outPath % QStringLiteral("/") % inputFile.fileName() %
                Kryvo::Constants::kDot %
                Kryvo::Constants::kCompressedFileExtension);

      auto compressFunction =
        [this, q, inFilePath, compressedFilePath](std::size_t id) {
          emit q->compressFile(id, QFileInfo(inFilePath),
                               QFileInfo(compressedFilePath));
        };

      pipeline.stages.push_back(compressFunction);

      const QString& encryptedFilePath =
        QString(compressedFilePath % Kryvo::Constants::kDot %
                Kryvo::Constants::kEncryptedFileExtension);

      auto encryptFunction =
        [this, q, passphrase, compressedFilePath, encryptedFilePath, cipher,
         keySize, modeOfOperation, compress](std::size_t id) {
          emit q->encryptFile(id, passphrase, QFileInfo(compressedFilePath),
                              QFileInfo(encryptedFilePath), cipher, keySize,
                              modeOfOperation, compress);
        };

      pipeline.stages.push_back(encryptFunction);

      if (removeIntermediateFiles) {
        auto removeIntermediateFilesFunction =
          [this, q, compressedFilePath](std::size_t id) {
            QFile::remove(compressedFilePath);
            emit q->fileCompleted(id);
          };

        pipeline.stages.push_back(removeIntermediateFilesFunction);
      }
    } else {
      const QString& encryptedFilePath =
        QString(outPath % QStringLiteral("/") % inputFile.fileName() %
                Kryvo::Constants::kDot %
                Kryvo::Constants::kEncryptedFileExtension);

      auto encryptFunction =
        [this, q, passphrase, inFilePath, encryptedFilePath, cipher, keySize,
         modeOfOperation, compress](std::size_t id) {
          emit q->encryptFile(id, passphrase, QFileInfo(inFilePath),
                              QFileInfo(encryptedFilePath), cipher, keySize,
                              modeOfOperation, compress);
        };

      pipeline.stages.push_back(encryptFunction);
    }

    pipelines.push_back(pipeline);
  }

  state.init(id);

  dispatch();
}

void Kryvo::DispatcherPrivate::decrypt(const QString& passphrase,
                                       const std::vector<QFileInfo>& inputFiles,
                                       const QDir& outputPath,
                                       const bool removeIntermediateFiles) {
  Q_Q(Dispatcher);

  state.busy(true);
  emit q->busyStatus(state.isBusy());

  pipelines.clear();

  std::size_t id = 0;

  for (const QFileInfo& inputFile : inputFiles) {
    const QString& inFilePath = inputFile.absoluteFilePath();

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

    if (headerString != QByteArrayLiteral("-------- Encrypted File --------")) {
      emit q->errorMessage(Constants::messages[7], inFilePath);
    }

    const QString& fileVersionString = readLine(&inFile);

    bool conversionOk = false;

    const int fileVersion = fileVersionString.toInt(&conversionOk);

    QString algorithmNameString;
    QString keySizeString;
    QString compressString;

    QString pbkdfSaltString;
    QString keySaltString;
    QString ivSaltString;

    if (1 == fileVersion) {
        const QString& providerString = readLine(&inFile);

        if (QStringLiteral("Botan") == providerString) {
          algorithmNameString = readLine(&inFile);
          keySizeString = readLine(&inFile);
          compressString = readLine(&inFile);

          pbkdfSaltString = readLine(&inFile);
          keySaltString = readLine(&inFile);
          ivSaltString = readLine(&inFile);
        }
    }

    const QByteArray& footerString = readLine(&inFile);

    if (footerString !=
        QByteArrayLiteral("---------------------------------")) {
      emit q->errorMessage(Constants::messages[7], inFilePath);
    }

    // Create output path if it doesn't exist
    if (!outputPath.exists()) {
      outputPath.mkpath(outputPath.absolutePath());
    }

    const QString& outPath = outputPath.exists() ?
                             outputPath.absolutePath() :
                             inputFile.absolutePath();

    const QString& outputFilePath = QString(outPath % QStringLiteral("/") %
                                            inputFile.fileName());

    Pipeline pipeline;

    pipeline.inputFilePath = inFilePath;

    id = id + 1;

    // Remove the .enc extensions if at the end of the file path
    const QString& decryptedFilePath =
      Constants::removeExtension(outputFilePath,
                                 Constants::kEncryptedFileExtension);

    // Create a unique file name for the file in this directory
    const QString& uniqueDecryptedFilePath =
      Constants::uniqueFilePath(decryptedFilePath);

    auto decryptFunction =
      [this, q, passphrase, inFilePath, uniqueDecryptedFilePath,
       algorithmNameString, keySizeString, pbkdfSaltString, keySaltString,
       ivSaltString](std::size_t id) {
        emit q->decryptFile(id, passphrase, inFilePath,
                            uniqueDecryptedFilePath, algorithmNameString,
                            keySizeString, pbkdfSaltString, keySaltString,
                            ivSaltString);
      };

    pipeline.stages.push_back(decryptFunction);

    if (QByteArrayLiteral("Gzip Compressed") == compressString) {
      // Remove the gz extension if at the end of the file path
      const QString& decompressedFilePath =
        Constants::removeExtension(uniqueDecryptedFilePath,
                                   Constants::kCompressedFileExtension);

      // Create a unique file name for the file in this directory
      const QString& uniqueDecompressedFilePath =
        Constants::uniqueFilePath(decompressedFilePath);

      auto decompressFunction =
        [this, q, uniqueDecryptedFilePath,
         uniqueDecompressedFilePath](std::size_t id) {
          emit q->decompressFile(id, QFileInfo(uniqueDecryptedFilePath),
                                 QFileInfo(uniqueDecompressedFilePath));
        };

      pipeline.stages.push_back(decompressFunction);

      if (removeIntermediateFiles) {
        auto removeIntermediateFilesFunction =
          [this, q, uniqueDecryptedFilePath](std::size_t id) {
            QFile::remove(uniqueDecryptedFilePath);
            emit q->fileCompleted(id);
          };

        pipeline.stages.push_back(removeIntermediateFilesFunction);
      }
    }

    pipelines.push_back(pipeline);
  }

  state.init(id);

  dispatch();
}

Kryvo::Dispatcher::Dispatcher(QObject* parent)
  : QObject(parent), d_ptr(std::make_unique<DispatcherPrivate>(this)) {
}

Kryvo::Dispatcher::~Dispatcher() = default;

void Kryvo::Dispatcher::encrypt(const QString& passphrase,
                                const std::vector<QFileInfo>& inputFiles,
                                const QDir& outputPath,
                                const QString& cipher,
                                const std::size_t inputKeySize,
                                const QString& modeOfOperation,
                                const bool compress,
                                const bool removeIntermediateFiles) {
  Q_D(Dispatcher);

  d->encrypt(passphrase, inputFiles, outputPath, cipher, inputKeySize,
             modeOfOperation, compress, removeIntermediateFiles);
}

void Kryvo::Dispatcher::decrypt(const QString& passphrase,
                                const std::vector<QFileInfo>& inputFiles,
                                const QDir& outputPath,
                                const bool removeIntermediateFiles) {
  Q_D(Dispatcher);

  d->decrypt(passphrase, inputFiles, outputPath, removeIntermediateFiles);
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

void Kryvo::Dispatcher::stop(const QFileInfo& fileInfo) {
  Q_D(Dispatcher);

  if (d->state.isBusy()) {
    std::size_t id = 0;
    bool found = false;

    for (std::size_t i = 0; i < d->pipelines.size(); ++i) {
      const Pipeline& pipeline = d->pipelines.at(i);

      if (pipeline.inputFilePath == fileInfo) {
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
