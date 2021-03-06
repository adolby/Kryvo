#include "Scheduler.hpp"
#include "SchedulerState.hpp"
#include "PluginLoader.hpp"
#include "archive/Archiver.hpp"
#include "cryptography/Crypto.hpp"
#include "FileUtility.h"
#include "Constants.hpp"
#include "utility/Thread.hpp"
#include <QSaveFile>
#include <QTimer>
#include <QStringBuilder>

class Kryvo::SchedulerPrivate {
  Q_DISABLE_COPY(SchedulerPrivate)
  Q_DECLARE_PUBLIC(Scheduler)

 public:
  explicit SchedulerPrivate(Scheduler* disp);

  void init();

  bool isComplete() const;

  void dispatch();

  void processPipeline(std::size_t id);

  void abortPipeline(std::size_t id);

  void encrypt(const QString& cryptoProvider,
               const QString& compressionFormat,
               const QString& passphrase,
               const std::vector<QFileInfo>& inputFiles,
               const QDir& outputPath,
               const QString& cipher,
               std::size_t inputKeySize,
               const QString& modeOfOperation,
               bool removeIntermediateFiles);

  void decrypt(const QString& passphrase,
               const std::vector<QFileInfo>& inputFilePaths,
               const QDir& outputPath,
               bool removeIntermediateFiles);

  Scheduler* const q_ptr{nullptr};

  SchedulerState state;

  PluginLoader pluginLoader;

  Archiver archiver;
  Crypto cryptographer;

  std::vector<Pipeline> pipelines;
};

Kryvo::SchedulerPrivate::SchedulerPrivate(Scheduler* s)
  : q_ptr(s), archiver(&state), cryptographer(&state) {
  QObject::connect(q_ptr, &Scheduler::fileCompleted,
                   q_ptr, &Scheduler::processPipeline,
                   Qt::QueuedConnection);

  QObject::connect(&pluginLoader, &PluginLoader::cryptoProvidersChanged,
                   &cryptographer, &Crypto::updateProviders,
                   Qt::QueuedConnection);

  QObject::connect(&archiver, &Archiver::fileProgress,
                   q_ptr, &Scheduler::updateFileProgress,
                   Qt::QueuedConnection);

  QObject::connect(&archiver, &Archiver::fileCompleted,
                   q_ptr, &Scheduler::fileCompleted,
                   Qt::QueuedConnection);

  QObject::connect(&archiver, &Archiver::fileFailed,
                   q_ptr, &Scheduler::abortPipeline,
                   Qt::QueuedConnection);

  QObject::connect(&archiver, &Archiver::statusMessage,
                   q_ptr, &Scheduler::statusMessage,
                   Qt::QueuedConnection);

  QObject::connect(&archiver, &Archiver::errorMessage,
                   q_ptr, &Scheduler::errorMessage,
                   Qt::QueuedConnection);

  QObject::connect(q_ptr, &Scheduler::compressFile,
                   &archiver, &Archiver::compress,
                   Qt::DirectConnection);

  QObject::connect(q_ptr, &Scheduler::decompressFile,
                   &archiver, &Archiver::decompress,
                   Qt::DirectConnection);

  QObject::connect(&cryptographer, &Crypto::fileProgress,
                   q_ptr, &Scheduler::updateFileProgress,
                   Qt::QueuedConnection);

  QObject::connect(&cryptographer, &Crypto::fileCompleted,
                   q_ptr, &Scheduler::fileCompleted,
                   Qt::QueuedConnection);

  QObject::connect(&cryptographer, &Crypto::fileFailed,
                   q_ptr, &Scheduler::abortPipeline,
                   Qt::QueuedConnection);

  QObject::connect(&cryptographer, &Crypto::statusMessage,
                   q_ptr, &Scheduler::statusMessage,
                   Qt::QueuedConnection);

  QObject::connect(&cryptographer, &Crypto::errorMessage,
                   q_ptr, &Scheduler::errorMessage,
                   Qt::QueuedConnection);

  QObject::connect(q_ptr, &Scheduler::encryptFile,
                   &cryptographer, &Crypto::encrypt,
                   Qt::DirectConnection);

  QObject::connect(q_ptr, &Scheduler::decryptFile,
                   &cryptographer, &Crypto::decrypt,
                   Qt::DirectConnection);

  QTimer::singleShot(0, q_ptr,
                     [this]() {
                       init();
                     });
}

void Kryvo::SchedulerPrivate::init() {
  pluginLoader.loadPlugins();
}

bool Kryvo::SchedulerPrivate::isComplete() const {
  Q_Q(const Scheduler);

  bool finished = true;

  for (const Pipeline& pipeline : pipelines) {
    if (pipeline.stage < pipeline.stages.size()) {
      finished = false;
      break;
    }
  }

  return finished;
}

void Kryvo::SchedulerPrivate::dispatch() {
  Q_Q(Scheduler);

  for (std::size_t i = 0; i < pipelines.size(); ++i) {
   processPipeline(i);
  }
}

void Kryvo::SchedulerPrivate::processPipeline(const std::size_t id) {
  Q_Q(Scheduler);
  Q_ASSERT(id < pipelines.size());

  if (id >= pipelines.size()) { // Safety check
    state.running(false);
    emit q->running(false);
    emit q->errorMessage(Constants::kMessages[0], QFileInfo());
    return;
  }

  const bool complete = isComplete();

  if (complete) {
    state.running(false);
    emit q->running(false);
    return;
  }

  Pipeline pipeline = pipelines.at(id);

  if (pipeline.stage >= pipeline.stages.size()) {
    return;
  }

  const std::function<void(std::size_t)> func =
    pipeline.stages.at(pipeline.stage);

  pipeline.stage = pipeline.stage + 1;

  pipelines[id] = pipeline;

  func(id);
}

void Kryvo::SchedulerPrivate::abortPipeline(const std::size_t id) {
  Q_Q(Scheduler);
  Q_ASSERT(id < pipelines.size());

  if (id >= pipelines.size()) { // Safety check
    state.running(false);
    emit q->running(false);
    emit q->errorMessage(Constants::kMessages[0], QFileInfo());
    return;
  }

  Pipeline pipeline = pipelines.at(id);

  pipeline.stage = pipeline.stages.size();

  pipelines[id] = pipeline;

  const bool complete = isComplete();

  if (complete) {
    state.running(false);
    emit q->running(false);
  }
}

void Kryvo::SchedulerPrivate::encrypt(const QString& cryptoProvider,
                                      const QString& compressionFormat,
                                      const QString& passphrase,
                                      const std::vector<QFileInfo>& inputFiles,
                                      const QDir& outputPath,
                                      const QString& cipher,
                                      const std::size_t inputKeySize,
                                      const QString& modeOfOperation,
                                      const bool removeIntermediateFiles) {
  Q_Q(Scheduler);

  if (state.isRunning()) {
    emit q->errorMessage(Constants::kMessages[12], QFileInfo());
    return;
  }

  state.running(true);
  emit q->running(true);

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

    const QString outPath = outputPath.exists() ?
                            outputPath.absolutePath() :
                            inputFile.absolutePath();

    Pipeline pipeline;

    pipeline.inputFilePath = inputFile;

    id = id + 1;

    if (QStringLiteral("gzip") == compressionFormat) {
      const QString compressedFilePath =
        QString(outPath % QStringLiteral("/") % inputFile.fileName() %
                Kryvo::Constants::kDot %
                Kryvo::Constants::kCompressedFileExtension);

      auto compressFunction =
        [this, q, inputFile, compressedFilePath](std::size_t id) {
          emit q->compressFile(id, inputFile, QFileInfo(compressedFilePath));
        };

      pipeline.stages.push_back(compressFunction);

      const QString encryptedFilePath =
        QString(compressedFilePath % Kryvo::Constants::kDot %
                Kryvo::Constants::kEncryptedFileExtension);

      auto encryptFunction =
        [this, q, cryptoProvider, compressionFormat, passphrase,
         compressedFilePath, encryptedFilePath, cipher, keySize,
         modeOfOperation](std::size_t id) {
          emit q->encryptFile(id, cryptoProvider, compressionFormat,
                              passphrase, QFileInfo(compressedFilePath),
                              QFileInfo(encryptedFilePath), cipher, keySize,
                              modeOfOperation);
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
      const QString encryptedFilePath =
        QString(outPath % QStringLiteral("/") % inputFile.fileName() %
                Kryvo::Constants::kDot %
                Kryvo::Constants::kEncryptedFileExtension);

      auto encryptFunction =
        [this, q, cryptoProvider, compressionFormat, passphrase, inputFile,
         encryptedFilePath, cipher, keySize, modeOfOperation](std::size_t id) {
          emit q->encryptFile(id, cryptoProvider, compressionFormat,
                              passphrase, inputFile,
                              QFileInfo(encryptedFilePath), cipher, keySize,
                              modeOfOperation);
        };

      pipeline.stages.push_back(encryptFunction);
    }

    pipelines.push_back(pipeline);
  }

  if (pipelines.empty()) { // No valid files to process
    state.running(false);
    emit q->running(false);
    return;
  }

  state.init(id);

  dispatch();
}

void Kryvo::SchedulerPrivate::decrypt(const QString& passphrase,
                                      const std::vector<QFileInfo>& inputFiles,
                                      const QDir& outputPath,
                                      const bool removeIntermediateFiles) {
  Q_Q(Scheduler);

  if (state.isRunning()) {
    emit q->errorMessage(Constants::kMessages[12], QFileInfo());
    return;
  }

  state.running(true);
  emit q->running(true);

  pipelines.clear();

  std::size_t id = 0;

  for (const QFileInfo& inputFile : inputFiles) {
    QFile inFile(inputFile.absoluteFilePath());

    const bool inFileOpen = inFile.open(QIODevice::ReadOnly);

    if (!inFileOpen) {
      emit q->errorMessage(Constants::kMessages[5], inputFile);
      continue;
    }

    const QHash<QByteArray, QByteArray> header = readHeader(&inFile);

    if (!header.contains(QByteArrayLiteral("Version"))) {
      emit q->errorMessage(Kryvo::Constants::kMessages[7], inputFile);
      continue;
    }

    const QByteArray cryptoProvider =
      header.value(QByteArrayLiteral("Cryptography provider"));

    const QByteArray compressionFormat =
      header.value(QByteArrayLiteral("Compression format"));

    // Create output path if it doesn't exist
    if (!outputPath.exists()) {
      outputPath.mkpath(outputPath.absolutePath());
    }

    const QString outPath = outputPath.exists() ?
                            outputPath.absolutePath() :
                            inputFile.absolutePath();

    const QString outputFilePath = QString(outPath % QStringLiteral("/") %
                                           inputFile.fileName());

    Pipeline pipeline;

    pipeline.inputFilePath = inputFile;

    id = id + 1;

    // Remove the .enc extensions if at the end of the file path
    const QString decryptedFilePath =
      removeExtension(outputFilePath, Constants::kEncryptedFileExtension);

    // Create a unique file name for the file in this directory
    const QFileInfo uniqueDecryptedFilePath(uniqueFilePath(decryptedFilePath));

    auto decryptFunction =
      [this, q, cryptoProvider, passphrase, inputFile, uniqueDecryptedFilePath,
       header](std::size_t id) {
        emit q->decryptFile(id, cryptoProvider, passphrase, inputFile,
                            uniqueDecryptedFilePath);
      };

    pipeline.stages.push_back(decryptFunction);

    if (QByteArrayLiteral("gzip") == compressionFormat) {
      // Remove the gz extension if at the end of the file path
      const QString decompressedFilePath =
        removeExtension(uniqueDecryptedFilePath.absoluteFilePath(),
                        Constants::kCompressedFileExtension);

      // Create a unique file name for the file in this directory
      const QFileInfo uniqueDecompressedFilePath(
        uniqueFilePath(decompressedFilePath));

      auto decompressFunction =
        [this, q, uniqueDecryptedFilePath,
         uniqueDecompressedFilePath](std::size_t id) {
          emit q->decompressFile(id, uniqueDecryptedFilePath,
                                 uniqueDecompressedFilePath);
        };

      pipeline.stages.push_back(decompressFunction);

      if (removeIntermediateFiles) {
        auto removeIntermediateFilesFunction =
          [this, q, uniqueDecryptedFilePath](std::size_t id) {
            QFile::remove(uniqueDecryptedFilePath.absoluteFilePath());
            emit q->fileCompleted(id);
          };

        pipeline.stages.push_back(removeIntermediateFilesFunction);
      }
    }

    pipelines.push_back(pipeline);
  }

  if (pipelines.empty()) { // No valid files to process
    state.running(false);
    return;
  }

  state.init(id);

  dispatch();
}

Kryvo::Scheduler::Scheduler(QObject* parent)
  : QObject(parent), d_ptr(std::make_unique<SchedulerPrivate>(this)) {
}

Kryvo::Scheduler::~Scheduler() = default;

void Kryvo::Scheduler::encrypt(const QString& cryptoProvider,
                               const QString& compressionFormat,
                               const QString& passphrase,
                               const std::vector<QFileInfo>& inputFiles,
                               const QDir& outputPath,
                               const QString& cipher,
                               const std::size_t inputKeySize,
                               const QString& modeOfOperation,
                               const bool removeIntermediateFiles) {
  Q_D(Scheduler);

  d->encrypt(cryptoProvider, compressionFormat, passphrase, inputFiles,
             outputPath, cipher, inputKeySize, modeOfOperation,
             removeIntermediateFiles);
}

void Kryvo::Scheduler::decrypt(const QString& passphrase,
                               const std::vector<QFileInfo>& inputFiles,
                               const QDir& outputPath,
                               const bool removeIntermediateFiles) {
  Q_D(Scheduler);

  d->decrypt(passphrase, inputFiles, outputPath, removeIntermediateFiles);
}

void Kryvo::Scheduler::abort() {
  Q_D(Scheduler);

  if (d->state.isRunning()) {
    d->state.abort(true);
  }
}

void Kryvo::Scheduler::pause(const bool pause) {
  Q_D(Scheduler);

  d->state.pause(pause);
}

void Kryvo::Scheduler::stop(const QFileInfo& fileInfo) {
  Q_D(Scheduler);

  if (d->state.isRunning()) {
    std::size_t id = 0;
    bool found = false;

    for (std::size_t i = 0; i < d->pipelines.size(); ++i) {
      const Pipeline pipeline = d->pipelines.at(i);

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

void Kryvo::Scheduler::processPipeline(const std::size_t id) {
  Q_D(Scheduler);

  d->processPipeline(id);
}

void Kryvo::Scheduler::abortPipeline(const std::size_t id) {
  Q_D(Scheduler);

  d->abortPipeline(id);
}

void Kryvo::Scheduler::updateFileProgress(const std::size_t id,
                                          const QString& task,
                                          const qint64 percentProgress) {
  Q_D(Scheduler);

  if (id >= d->pipelines.size()) {
    return;
  }

  const Pipeline pipeline = d->pipelines.at(id);

  emit fileProgress(pipeline.inputFilePath, task, percentProgress);
}
