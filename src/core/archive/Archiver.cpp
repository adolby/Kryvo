#include "archive/Archiver.hpp"
#include "Constants.hpp"
#include "DispatchQueue.hpp"
#include <QDir>
#include <QSaveFile>
#include <QFile>
#include <QStringBuilder>
#include "zlib.h"

class Kryvo::ArchiverPrivate {
  Q_DISABLE_COPY(ArchiverPrivate)
  Q_DECLARE_PUBLIC(Archiver)

 public:
  ArchiverPrivate(Archiver* archiver, SchedulerState* s);

  int gzipDeflateFile(std::size_t id, QFile* source, QSaveFile* dest,
                      int level = Z_DEFAULT_COMPRESSION);
  int gzipInflateFile(std::size_t id, QFile* source, QSaveFile* dest);
  QByteArray compressChunk(const QByteArray& chunk);
  bool compressFile(std::size_t id, const QFileInfo& inputFileInfo,
                    const QFileInfo& outputFileInfo);
  bool decompressFile(std::size_t id, const QFileInfo& inputFileInfo,
                      const QFileInfo& outFileInfo);
  void compress(std::size_t id, const QFileInfo& inputFileInfo,
                const QFileInfo& outputFileInfo);
  void decompress(std::size_t id, const QFileInfo& inputFileInfo,
                  const QFileInfo& outputFileInfo);

  Archiver* const q_ptr{nullptr};

  SchedulerState* state{nullptr};

  DispatchQueue queue;

  static constexpr qint64 kChunk{256000};
};

Kryvo::ArchiverPrivate::ArchiverPrivate(Archiver* archiver, SchedulerState* s)
  : q_ptr(archiver), state(s) {
}

/* Compress from file source to file dest until EOF on source.
   Returns Z_OK on success, Z_MEM_ERROR if memory could not be
   allocated for processing, Z_STREAM_ERROR if an invalid compression
   level is supplied, Z_VERSION_ERROR if the version of zlib.h and the
   version of the library linked do not match, or Z_ERRNO if there is
   an error reading or writing the files. */
int Kryvo::ArchiverPrivate::gzipDeflateFile(const std::size_t id, QFile* source,
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

  QByteArray in;
  in.resize(kChunk);

  QByteArray out;
  out.resize(kChunk);

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
      dest->cancelWriting();
      return Z_ERRNO;
    }

    const qint64 bytesRead = source->read(in.data(), kChunk);

    if (bytesRead < 0) {
      deflateEnd(&strm);
      dest->cancelWriting();
      return Z_ERRNO;
    }

    strm.avail_in = bytesRead;

    flush = source->atEnd() ? Z_FINISH : Z_NO_FLUSH;

    strm.next_in = reinterpret_cast<unsigned char*>(in.data());

    /* run deflate() on input until output buffer not full, finish
       compression if all of source has been read in */
    do {
      state->pauseWait(id);

      strm.avail_out = kChunk;
      strm.next_out = reinterpret_cast<unsigned char*>(out.data());
      ret = deflate(&strm, flush); /* no bad return value */

      Q_ASSERT(ret != Z_STREAM_ERROR); /* state not clobbered */

      have = kChunk - strm.avail_out;

      const qint64 bytesWritten = dest->write(out.data(), have);

      if (bytesWritten != have || bytesWritten < 0) {
        deflateEnd(&strm);
        dest->cancelWriting();
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
int Kryvo::ArchiverPrivate::gzipInflateFile(const std::size_t id, QFile* source,
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

  QByteArray in;
  in.resize(kChunk);

  QByteArray out;
  out.resize(kChunk);

  /* allocate inflate state */
  strm.zalloc = Z_NULL;
  strm.zfree = Z_NULL;
  strm.opaque = Z_NULL;
  strm.avail_in = 0;
  strm.next_in = Z_NULL;

  ret = inflateInit2(&strm, MAX_WBITS + 16);

  if (ret != Z_OK) {
    dest->cancelWriting();
    return ret;
  }

  qint64 totalBytesRead = 0;

  /* decompress until deflate stream ends or end of file */
  do {
    state->pauseWait(id);

    if (state->isAborted() || state->isStopped(id)) {
      inflateEnd(&strm);
      dest->cancelWriting();
      return Z_ERRNO;
    }

    const qint64 bytesRead = source->read(in.data(), kChunk);

    if (bytesRead < 0) {
      inflateEnd(&strm);
      dest->cancelWriting();
      return Z_ERRNO;
    }

    strm.avail_in = bytesRead;

    if (0 == strm.avail_in) {
      break;
    }

    strm.next_in = reinterpret_cast<unsigned char*>(in.data());

    /* run inflate() on input until output buffer not full */
    do {
      state->pauseWait(id);

      strm.avail_out = kChunk;
      strm.next_out = reinterpret_cast<unsigned char*>(out.data());
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
          dest->cancelWriting();
          return ret;
        }
      }

      have = kChunk - strm.avail_out;

      const qint64 bytesWritten = dest->write(out.data(), have);

      if (bytesWritten != have || bytesWritten < 0) {
        inflateEnd(&strm);
        dest->cancelWriting();
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

QByteArray Kryvo::ArchiverPrivate::compressChunk(const QByteArray& chunk) {
    return QByteArray();
}

bool Kryvo::ArchiverPrivate::compressFile(const std::size_t id,
                                          const QFileInfo& inputFileInfo,
                                          const QFileInfo& outputFileInfo) {
  Q_Q(Archiver);
  Q_ASSERT(state);

  if (!state) {
    emit q->errorMessage(Constants::kMessages[0], QFileInfo());
    emit q->fileFailed(id);
    return false;
  }

  if (state->isAborted() || state->isStopped(id)) {
    emit q->fileFailed(id);
    return false;
  }

  if (!inputFileInfo.exists() || !inputFileInfo.isFile() ||
      !inputFileInfo.isReadable()) {
    emit q->errorMessage(Constants::kMessages[9],
                         inputFileInfo.absoluteFilePath());
    emit q->fileFailed(id);
    return false;
  }

  QFile inputFile(inputFileInfo.absoluteFilePath());
  const bool inputFileOpen = inputFile.open(QIODevice::ReadOnly);
  if (!inputFileOpen) {
    emit q->fileFailed(id);
    return false;
  }

  QSaveFile outputFile(outputFileInfo.absoluteFilePath());
  const bool outputFileOpen = outputFile.open(QIODevice::WriteOnly);
  if (!outputFileOpen) {
    emit q->fileFailed(id);
    return false;
  }

  const int ret = gzipDeflateFile(id, &inputFile, &outputFile,
                                  Z_DEFAULT_COMPRESSION);

  if (ret != Z_OK) {
    emit q->errorMessage(Constants::kMessages[9], inputFileInfo);
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

bool Kryvo::ArchiverPrivate::decompressFile(const std::size_t id,
                                            const QFileInfo& inputFileInfo,
                                            const QFileInfo& outputFileInfo) {
  Q_Q(Archiver);
  Q_ASSERT(state);

  if (!state) {
    emit q->errorMessage(Constants::kMessages[0], QFileInfo());
    emit q->fileFailed(id);
    return false;
  }

  if (state->isAborted() || state->isStopped(id)) {
    emit q->fileFailed(id);
    return false;
  }

  if (!inputFileInfo.exists() || !inputFileInfo.isFile() ||
      !inputFileInfo.isReadable()) {
    emit q->errorMessage(Constants::kMessages[10], inputFileInfo);
    emit q->fileFailed(id);
    return false;
  }

  QFile inputFile(inputFileInfo.absoluteFilePath());
  const bool inputFileOpen = inputFile.open(QIODevice::ReadOnly);
  if (!inputFileOpen) {
    emit q->fileFailed(id);
    return false;
  }

  QSaveFile outputFile(outputFileInfo.absoluteFilePath());
  const bool outputFileOpen = outputFile.open(QIODevice::WriteOnly);
  if (!outputFileOpen) {
    emit q->fileFailed(id);
    return false;
  }

  const int ret = gzipInflateFile(id, &inputFile, &outputFile);

  if (ret != Z_OK) {
    emit q->errorMessage(Constants::kMessages[10], inputFileInfo);
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

void Kryvo::ArchiverPrivate::compress(const std::size_t id,
                                      const QFileInfo& inputFileInfo,
                                      const QFileInfo& outputFileInfo) {
  const auto compressFunc =
    [this, id, inputFileInfo, outputFileInfo]() {
      compressFile(id, inputFileInfo, outputFileInfo);
    };

  DispatchTask task;
  task.func = compressFunc;

  queue.enqueue(task);
}

void Kryvo::ArchiverPrivate::decompress(const std::size_t id,
                                        const QFileInfo& inputFileInfo,
                                        const QFileInfo& outputFileInfo) {
  const auto decompressFunc =
    [this, id, inputFileInfo, outputFileInfo]() {
      decompressFile(id, inputFileInfo, outputFileInfo);
    };

  DispatchTask task;
  task.func = decompressFunc;

  queue.enqueue(task);
}

Kryvo::Archiver::Archiver(SchedulerState* state, QObject* parent)
  : Pipe(parent), d_ptr(std::make_unique<ArchiverPrivate>(this, state)) {
}

Kryvo::Archiver::~Archiver() = default;

bool Kryvo::Archiver::compressFile(const std::size_t id,
                                   const QFileInfo& inputFileInfo,
                                   const QFileInfo& outputFileInfo) {
  Q_D(Archiver);

  return d->compressFile(id, inputFileInfo, outputFileInfo);
}

bool Kryvo::Archiver::decompressFile(const std::size_t id,
                                     const QFileInfo& inputFileInfo,
                                     const QFileInfo& outputFileInfo) {
  Q_D(Archiver);

  return d->decompressFile(id, inputFileInfo, outputFileInfo);
}

void Kryvo::Archiver::compress(const std::size_t id,
                               const QFileInfo& inputFileInfo,
                               const QFileInfo& outputFileInfo) {
  Q_D(Archiver);

  d->compress(id, inputFileInfo, outputFileInfo);
}

void Kryvo::Archiver::decompress(const std::size_t id,
                                 const QFileInfo& inputFileInfo,
                                 const QFileInfo& outputFileInfo) {
  Q_D(Archiver);

  d->decompress(id, inputFileInfo, outputFileInfo);
}

//void Kryvo::Archiver::archive(const std::vector<QFileInfo>& inputFiles) {
//  // TODO: Adapt a tar implementation
//}

//void Kryvo::Archiver::extract(const QFileInfo& archiveFileInfo) {
//  // TODO: Adapt a tar implementation
//}
