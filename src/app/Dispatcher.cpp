#include "Dispatcher.hpp"
#include "DispatcherState.hpp"
#include "archive/Archiver.hpp"
#include "cryptography/Crypto.hpp"
#include "Constants.hpp"
#include "utility/Thread.hpp"
#include <QCoreApplication>

class Kryvo::DispatcherPrivate {
  Q_DISABLE_COPY(DispatcherPrivate)

 public:
  DispatcherPrivate();

  DispatcherState state;

  Archiver archiver;
  Thread archiverThread;
  Crypto cryptography;
  Thread cryptographyThread;
};

Kryvo::DispatcherPrivate::DispatcherPrivate()
  : archiver(&state), cryptography(&state) {
  archiver.moveToThread(&archiverThread);
  cryptography.moveToThread(&cryptographyThread);

  archiverThread.start();
  cryptographyThread.start();
}

Kryvo::Dispatcher::Dispatcher(QObject* parent)
  : QObject(parent), d_ptr(std::make_unique<DispatcherPrivate>()) {
}

Kryvo::Dispatcher::~Dispatcher() = default;

void Kryvo::Dispatcher::encrypt(const QString& passphrase,
                                const QStringList& inputFilePaths,
                                const QString& outputPath,
                                const QString& cipher,
                                std::size_t inputKeySize,
                                const QString& modeOfOperation,
                                bool compress) {
  Q_D(Dispatcher);

  d->state.busy(true);
  emit busyStatus(d->state.isBusy());

  for (const QString& inputFilePath : inputFilePaths) {
    if (compress) {
      emit compressFile(inputFilePath, outputPath);
    }
    else {
      emit encryptFile(passphrase, inputFilePath, outputPath, cipher,
                       inputKeySize, modeOfOperation, compress);
    }
  }

  d->state.reset();

  d->state.busy(false);
  emit busyStatus(d->state.isBusy());
}

void Kryvo::Dispatcher::decrypt(const QString& passphrase,
                                const QStringList& inputFilePaths,
                                const QString& outputPath) {
  Q_D(Dispatcher);

  d->state.busy(true);
  emit busyStatus(d->state.isBusy());

  for (const QString& inputFilePath : inputFilePaths) {
    emit decryptFile(passphrase, inputFilePath, outputPath);
  }

  d->state.reset();

  d->state.busy(false);
  emit busyStatus(d->state.isBusy());
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
    d->state.stop(filePath, true);
  }
}
