#include "archive/Archiver.hpp"
#include "Constants.hpp"
#include "DispatchQueue.hpp"
#include <QDir>
#include <QSaveFile>
#include <QFile>
#include <QStringBuilder>
#include "zlib.h"

namespace Kryvo {

class ArchiverPrivate {
  Q_DISABLE_COPY(ArchiverPrivate)
  Q_DECLARE_PUBLIC(Archiver)

 public:
  ArchiverPrivate(Archiver* archiver, SchedulerState* s);

  int gzipDeflateFile(std::size_t id, QFile* source, QSaveFile* dest,
                      int level = Z_DEFAULT_COMPRESSION);
  int gzipInflateFile(std::size_t id, QFile* source, QSaveFile* dest);
  bool compressFile(std::size_t id, const Kryvo::CompressFileConfig& config);
  bool decompressFile(std::size_t id,
                      const Kryvo::DecompressFileConfig& config);
  void compress(std::size_t id, const Kryvo::CompressFileConfig& config);
  void decompress(std::size_t id, const Kryvo::DecompressFileConfig& config);

  Archiver* const q_ptr{nullptr};

  SchedulerState* state{nullptr};

  DispatchQueue queue;

  static constexpr qint64 chunkSize{256000};
};

ArchiverPrivate::ArchiverPrivate(Archiver* archiver, SchedulerState* s)
  : q_ptr(archiver), state(s) {
}

/* Compress from file source to file dest until EOF on source.
   Returns Z_OK on success, Z_MEM_ERROR if memory could not be
   allocated for processing, Z_STREAM_ERROR if an invalid compression
   level is supplied, Z_VERSION_ERROR if the version of zlib.h and the
   version of the library linked do not match, or Z_ERRNO if there is
   an error reading or writing the files. */
int ArchiverPrivate::gzipDeflateFile(const std::size_t id, QFile* source,
                                     QSaveFile* dest, int level) {
  Q_Q(Archiver);
  Q_ASSERT(state);
  Q_ASSERT(source);
  Q_ASSERT(dest);

  int ret = Z_ERRNO;

  if (!state) {
    return ret;
  }

  const qint64 totalBytes = source->size();

  int flush = 0;
  unsigned have = 0;

  QByteArray inBuffer(chunkSize, Qt::Uninitialized);
  QByteArray outBuffer(chunkSize, Qt::Uninitialized);

  /* allocate deflate state */
  z_stream strm;
  strm.zalloc = Z_NULL;
  strm.zfree = Z_NULL;
  strm.opaque = Z_NULL;

  ret = deflateInit2(&strm, level, Z_DEFLATED, MAX_WBITS + 16, 8,
                     Z_DEFAULT_STRATEGY);

  if (ret != Z_OK) {
    return ret;
  }

  qint64 totalBytesRead = 0;

  /* compress until end of file */
  do {
    state->pauseWait(id);

    if (state->isAborted() || state->isStopped(id)) {
      deflateEnd(&strm);
      return Z_ERRNO;
    }

    const qint64 bytesRead = source->read(inBuffer.data(), chunkSize);

    if (bytesRead < 0) {
      deflateEnd(&strm);
      return Z_ERRNO;
    }

    strm.avail_in = bytesRead;

    flush = source->atEnd() ? Z_FINISH : Z_NO_FLUSH;

    strm.next_in = reinterpret_cast<unsigned char*>(inBuffer.data());

    /* run deflate() on input until output buffer not full, finish
       compression if all of source has been read in */
    do {
      state->pauseWait(id);

      strm.avail_out = chunkSize;
      strm.next_out = reinterpret_cast<unsigned char*>(outBuffer.data());
      ret = deflate(&strm, flush); /* no bad return value */

      Q_ASSERT(ret != Z_STREAM_ERROR); /* state not clobbered */

      have = chunkSize - strm.avail_out;

      const qint64 bytesWritten = dest->write(outBuffer.data(), have);

      if (bytesWritten != have || bytesWritten < 0) {
        deflateEnd(&strm);
        return Z_ERRNO;
      }
    } while (0 == strm.avail_out);

    totalBytesRead = totalBytesRead + bytesRead;

    const double fractionalProgress =
      static_cast<double>(totalBytesRead) /
      static_cast<double>(totalBytes);

    const double percentProgress = fractionalProgress * 100.0;

    const int percentProgressInteger = static_cast<int>(percentProgress);

    emit q->fileProgress(id, QObject::tr("Compressing"),
                         percentProgressInteger);

    Q_ASSERT(0 == strm.avail_in); /* all input will be used */

    /* done when last data in file processed */
  } while (flush != Z_FINISH);

  Q_ASSERT(ret == Z_STREAM_END); /* stream will be complete */

  /* clean up and return */
  deflateEnd(&strm);

  return Z_OK;
}

/* Decompress from file source to file dest until stream ends or EOF.
   Returns Z_OK on success, Z_MEM_ERROR if memory could not be
   allocated for processing, Z_DATA_ERROR if the deflate data is
   invalid or incomplete, Z_VERSION_ERROR if the version of zlib.h and
   the version of the library linked do not match, or Z_ERRNO if there
   is an error reading or writing the files. */
int ArchiverPrivate::gzipInflateFile(const std::size_t id, QFile* source,
                                     QSaveFile* dest) {
  Q_Q(Archiver);
  Q_ASSERT(state);
  Q_ASSERT(source);
  Q_ASSERT(dest);

  int ret = Z_ERRNO;

  if (!state) {
    return ret;
  }

  const qint64 totalBytes = source->size();

  unsigned have = 0;

  z_stream strm;

  QByteArray inBuffer(chunkSize, Qt::Uninitialized);
  QByteArray outBuffer(chunkSize, Qt::Uninitialized);

  /* allocate inflate state */
  strm.zalloc = Z_NULL;
  strm.zfree = Z_NULL;
  strm.opaque = Z_NULL;
  strm.avail_in = 0;
  strm.next_in = Z_NULL;

  ret = inflateInit2(&strm, MAX_WBITS + 16);

  if (ret != Z_OK) {
    return ret;
  }

  qint64 totalBytesRead = 0;

  /* decompress until deflate stream ends or end of file */
  do {
    state->pauseWait(id);

    if (state->isAborted() || state->isStopped(id)) {
      inflateEnd(&strm);
      return Z_ERRNO;
    }

    const qint64 bytesRead = source->read(inBuffer.data(), chunkSize);

    if (bytesRead < 0) {
      inflateEnd(&strm);
      return Z_ERRNO;
    }

    strm.avail_in = bytesRead;

    if (0 == strm.avail_in) {
      break;
    }

    strm.next_in = reinterpret_cast<unsigned char*>(inBuffer.data());

    /* run inflate() on input until output buffer not full */
    do {
      state->pauseWait(id);

      strm.avail_out = chunkSize;
      strm.next_out = reinterpret_cast<unsigned char*>(outBuffer.data());
      ret = inflate(&strm, Z_NO_FLUSH);

      Q_ASSERT(ret != Z_STREAM_ERROR); /* state not clobbered */

      switch (ret) {
        case Z_NEED_DICT: {
          ret = Z_DATA_ERROR;
          // Fall through
        }

        case Z_DATA_ERROR: {
          // Fall through
        }

        case Z_MEM_ERROR: {
          inflateEnd(&strm);
          return ret;
        }
      }

      have = chunkSize - strm.avail_out;

      const qint64 bytesWritten = dest->write(outBuffer.data(), have);

      if (bytesWritten != have || bytesWritten < 0) {
        inflateEnd(&strm);
        return Z_ERRNO;
      }
    } while (0 == strm.avail_out);

    totalBytesRead = totalBytesRead + bytesRead;

    const double fractionalProgress =
      static_cast<double>(totalBytesRead) /
      static_cast<double>(totalBytes);

    const double percentProgress = fractionalProgress * 100.0;

    const int percentProgressInteger = static_cast<int>(percentProgress);

    emit q->fileProgress(id, QObject::tr("Decompressing"),
                         percentProgressInteger);

    /* done when inflate() says it's done */
  } while (ret != Z_STREAM_END);

  /* clean up and return */
  inflateEnd(&strm);

  return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}

bool ArchiverPrivate::compressFile(const std::size_t id,
                                   const Kryvo::CompressFileConfig& config) {
  Q_Q(Archiver);
  Q_ASSERT(state);

  if (!state) {
    emit q->errorMessage(Constants::messages[0], QFileInfo());
    emit q->fileFailed(id);
    return false;
  }

  if (state->isAborted() || state->isStopped(id)) {
    emit q->fileFailed(id);
    return false;
  }

  if (!config.inputFileInfo.exists() || !config.inputFileInfo.isFile() ||
      !config.inputFileInfo.isReadable()) {
    emit q->errorMessage(Constants::messages[9],
                         QFileInfo(config.inputFileInfo.absoluteFilePath()));
    emit q->fileFailed(id);
    return false;
  }

  QFile inputFile(config.inputFileInfo.absoluteFilePath());
  const bool inputFileOpen = inputFile.open(QIODevice::ReadOnly);
  if (!inputFileOpen) {
    emit q->fileFailed(id);
    return false;
  }

  QSaveFile outputFile(config.outputFileInfo.absoluteFilePath());
  const bool outputFileOpen = outputFile.open(QIODevice::WriteOnly);
  if (!outputFileOpen) {
    emit q->fileFailed(id);
    return false;
  }

  const int ret = gzipDeflateFile(id, &inputFile, &outputFile,
                                  Z_DEFAULT_COMPRESSION);

  if (ret != Z_OK) {
    emit q->errorMessage(Constants::messages[9], config.inputFileInfo);
    emit q->fileFailed(id);
    return false;
  }

  if (state->isAborted() || state->isStopped(id)) {
    emit q->fileFailed(id);
    return false;
  }

  outputFile.commit();

  // Progress: finished
  emit q->fileProgress(id, QObject::tr("Compressed"), 100);

  emit q->fileCompleted(id);

  return true;
}

bool ArchiverPrivate::decompressFile(
  const std::size_t id, const Kryvo::DecompressFileConfig& config) {
  Q_Q(Archiver);
  Q_ASSERT(state);

  if (!state) {
    emit q->errorMessage(Constants::messages[0], QFileInfo());
    emit q->fileFailed(id);
    return false;
  }

  if (state->isAborted() || state->isStopped(id)) {
    emit q->fileFailed(id);
    return false;
  }

  if (!config.inputFileInfo.exists() || !config.inputFileInfo.isFile() ||
      !config.inputFileInfo.isReadable()) {
    emit q->errorMessage(Constants::messages[10], config.inputFileInfo);
    emit q->fileFailed(id);
    return false;
  }

  QFile inputFile(config.inputFileInfo.absoluteFilePath());
  const bool inputFileOpen = inputFile.open(QIODevice::ReadOnly);
  if (!inputFileOpen) {
    emit q->fileFailed(id);
    return false;
  }

  QSaveFile outputFile(config.outputFileInfo.absoluteFilePath());
  const bool outputFileOpen = outputFile.open(QIODevice::WriteOnly);
  if (!outputFileOpen) {
    emit q->fileFailed(id);
    return false;
  }

  const int ret = gzipInflateFile(id, &inputFile, &outputFile);

  if (ret != Z_OK) {
    emit q->errorMessage(Constants::messages[10], config.inputFileInfo);
    emit q->fileFailed(id);
    return false;
  }

  if (state->isAborted() || state->isStopped(id)) {
    emit q->fileFailed(id);
    return false;
  }

  outputFile.commit();

  // Progress: finished
  emit q->fileProgress(id, QObject::tr("Decompressed"), 100);

  emit q->fileCompleted(id);

  return true;
}

void ArchiverPrivate::compress(const std::size_t id,
                               const Kryvo::CompressFileConfig& config) {
  const auto compressFunc =
    [this, id, config]() {
      compressFile(id, config);
    };

  DispatchTask task;
  task.func = compressFunc;

  queue.enqueue(task);
}

void ArchiverPrivate::decompress(const std::size_t id,
                                 const Kryvo::DecompressFileConfig& config) {
  const auto decompressFunc =
    [this, id, config]() {
      decompressFile(id, config);
    };

  DispatchTask task;
  task.func = decompressFunc;

  queue.enqueue(task);
}

Archiver::Archiver(SchedulerState* state, QObject* parent)
  : Pipe(parent), d_ptr(std::make_unique<ArchiverPrivate>(this, state)) {
}

Archiver::~Archiver() = default;

bool Archiver::compressFile(const std::size_t id,
                            const Kryvo::CompressFileConfig& config) {
  Q_D(Archiver);

  return d->compressFile(id, config);
}

bool Archiver::decompressFile(const std::size_t id,
                              const Kryvo::DecompressFileConfig& config) {
  Q_D(Archiver);

  return d->decompressFile(id, config);
}

void Archiver::compress(const std::size_t id,
                        const Kryvo::CompressFileConfig& config) {
  Q_D(Archiver);

  d->compress(id, config);
}

void Archiver::decompress(const std::size_t id,
                          const Kryvo::DecompressFileConfig& config) {
  Q_D(Archiver);

  d->decompress(id, config);
}

//void Archiver::archive(const std::vector<QFileInfo>& inputFiles) {
//  // TODO: Adapt a tar implementation
//}

//void Archiver::extract(const QFileInfo& archiveFileInfo) {
//  // TODO: Adapt a tar implementation
//}

} // namespace Kryvo
