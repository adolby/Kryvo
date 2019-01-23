#include "archive/Archiver.hpp"
#include "Constants.hpp"
#include <QDir>
#include <QSaveFile>
#include <QFile>
#include <QStringBuilder>
#include "zlib.h"

#include <QDebug>

class Kryvo::ArchiverPrivate {
  Q_DISABLE_COPY(ArchiverPrivate)
  Q_DECLARE_PUBLIC(Archiver)

 public:
  ArchiverPrivate(Archiver* archiver, DispatcherState* ds);

  int gzipDeflateFile(std::size_t id, QFile* source, QSaveFile* dest,
                      int level = Z_DEFAULT_COMPRESSION);
  int gzipInflateFile(std::size_t id, QFile* source, QSaveFile* dest);
  QByteArray compressChunk(const QByteArray& chunk);
  bool compressFile(std::size_t id, const QString& inFilePath,
                    const QString& outFilePath);
  bool decompressFile(std::size_t id, const QString& inFilePath,
                      const QString& outFilePath);
  void compress(std::size_t id, const QString& inFilePath,
                const QString& outputPath);
  void decompress(std::size_t id, const QString& inFilePath,
                  const QString& outputPath);

  Archiver* const q_ptr{nullptr};

  DispatcherState* state{nullptr};

  static constexpr qint64 kChunk{16384};
};

Kryvo::ArchiverPrivate::ArchiverPrivate(Archiver* archiver,
                                        DispatcherState* ds)
  : q_ptr(archiver), state(ds) {
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

  qint64 totalBytesWritten = 0;

  /* compress until end of file */
  do {
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
      strm.avail_out = kChunk;
      strm.next_out = reinterpret_cast<unsigned char*>(out.data());
      ret = deflate(&strm, flush); /* no bad return value */

      Q_ASSERT(ret != Z_STREAM_ERROR); /* state not clobbered */

      have = kChunk - strm.avail_out;

      const qint64 bytesWritten = dest->write(out.data(), have);

      totalBytesWritten = totalBytesWritten + bytesWritten;

      const double fractionalProgress =
        static_cast<double>(totalBytesWritten) /
        static_cast<double>(totalBytes);

      const int percentProgress = fractionalProgress * 100;

      emit q->fileProgress(id, QObject::tr("Compressing"), percentProgress);

      if (bytesWritten != have || bytesWritten < 0) {
        deflateEnd(&strm);
        dest->cancelWriting();
        return Z_ERRNO;
      }
    } while (0 == strm.avail_out);

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
    return ret;
  }

  qint64 totalBytesWritten = 0;

  /* decompress until deflate stream ends or end of file */
  do {
    if (state->isAborted() || state->isStopped(id)) {
      deflateEnd(&strm);
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
      strm.avail_out = kChunk;
      strm.next_out = reinterpret_cast<unsigned char*>(out.data());
      ret = inflate(&strm, Z_NO_FLUSH);

      Q_ASSERT(ret != Z_STREAM_ERROR); /* state not clobbered */

      switch (ret) {
      case Z_NEED_DICT:
        ret = Z_DATA_ERROR; /* and fall through */
      case Z_DATA_ERROR:
      case Z_MEM_ERROR:
        inflateEnd(&strm);
        dest->cancelWriting();
        return ret;
      }

      have = kChunk - strm.avail_out;

      const qint64 bytesWritten = dest->write(out.data(), have);

      totalBytesWritten = totalBytesWritten + bytesWritten;

      const double fractionalProgress =
        static_cast<double>(totalBytesWritten / totalBytes);

      const int percentProgress = fractionalProgress * 100;

      emit q->fileProgress(id, QObject::tr("Decompressing"), percentProgress);

      if (bytesWritten != have || bytesWritten < 0) {
        inflateEnd(&strm);
        dest->cancelWriting();
        return Z_ERRNO;
      }
    } while (0 == strm.avail_out);

    /* done when inflate() says it's done */
  } while (ret != Z_STREAM_END && !state->isAborted() && !state->isStopped(id));

  /* clean up and return */
  inflateEnd(&strm);

  return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}

QByteArray Kryvo::ArchiverPrivate::compressChunk(const QByteArray& chunk) {
    return QByteArray();
}

bool Kryvo::ArchiverPrivate::compressFile(const std::size_t id,
                                          const QString& inputFilePath,
                                          const QString& outputFilePath) {
  Q_Q(Archiver);
  Q_ASSERT(state);

  if (!state) {
    emit q->errorMessage(Constants::messages[0], QString(""));
    emit q->fileFailed(id);
    return false;
  }

  if (state->isAborted() || state->isStopped(id)) {
    emit q->fileFailed(id);
    return false;
  }

  const QFileInfo inputFileInfo(inputFilePath);

  if (!inputFileInfo.exists() || !inputFileInfo.isFile() ||
      !inputFileInfo.isReadable()) {
    emit q->errorMessage(Constants::messages[9], inputFilePath);
    emit q->fileFailed(id);
    return false;
  }

  QFile source(inputFilePath);
  const bool sourceFileOpen = source.open(QIODevice::ReadOnly);
  if (!sourceFileOpen) {
    emit q->fileFailed(id);
    return false;
  }

  QSaveFile dest(outputFilePath);
  const bool destFileOpen = dest.open(QIODevice::WriteOnly);
  if (!destFileOpen) {
    emit q->fileFailed(id);
    return false;
  }

  const int ret = gzipDeflateFile(id, &source, &dest, Z_DEFAULT_COMPRESSION);

  if (ret != Z_OK) {
    emit q->errorMessage(Constants::messages[9], inputFilePath);
    emit q->fileFailed(id);
    return false;
  }

  if (state->isAborted() || state->isStopped(id)) {
    emit q->fileFailed(id);
    return false;
  }

  // Progress: finished
  emit q->fileProgress(id, QObject::tr("Compressed"), 100);

  dest.commit();

  emit q->fileCompleted(id);

  return true;
}

bool Kryvo::ArchiverPrivate::decompressFile(const std::size_t id,
                                            const QString& inputFilePath,
                                            const QString& outputFilePath) {
  Q_Q(Archiver);
  Q_ASSERT(state);

  if (!state) {
    emit q->errorMessage(Constants::messages[0], QString(""));
    emit q->fileFailed(id);
    return false;
  }

  if (state->isAborted() || state->isStopped(id)) {
    emit q->fileFailed(id);
    return false;
  }

  const QFileInfo inputFileInfo(inputFilePath);

  if (!inputFileInfo.exists() || !inputFileInfo.isFile() ||
      !inputFileInfo.isReadable()) {
    emit q->errorMessage(Constants::messages[10], inputFilePath);
    emit q->fileFailed(id);
    return false;
  }

  QFile source(inputFilePath);
  const bool sourceFileOpen = source.open(QIODevice::ReadOnly);
  if (!sourceFileOpen) {
    emit q->fileFailed(id);
    return false;
  }

  const qint64 totalBytes = source.size();

  QSaveFile dest(outputFilePath);
  const bool destFileOpen = dest.open(QIODevice::WriteOnly);
  if (!destFileOpen) {
    emit q->fileFailed(id);
    return false;
  }

  const int ret = gzipInflateFile(id, &source, &dest);

  if (ret != Z_OK) {
    emit q->errorMessage(Constants::messages[10], inputFilePath);
    emit q->fileFailed(id);
    return false;
  }

  if (state->isAborted() || state->isStopped(id)) {
    emit q->fileFailed(id);
    return false;
  }

  // Progress: finished
  emit q->fileProgress(id, QObject::tr("Decompressed"), 100);

  dest.commit();

  emit q->fileCompleted(id);

  return true;
}

void Kryvo::ArchiverPrivate::compress(const std::size_t id,
                                      const QString& inputFilePath,
                                      const QString& outputFilePath) {
  compressFile(id, inputFilePath, outputFilePath);
}

void Kryvo::ArchiverPrivate::decompress(const std::size_t id,
                                        const QString& inputFilePath,
                                        const QString& outputFilePath) {
  decompressFile(id, inputFilePath, outputFilePath);
}

Kryvo::Archiver::Archiver(DispatcherState* state, QObject* parent)
  : QObject(parent), d_ptr(std::make_unique<ArchiverPrivate>(this, state)) {
}

Kryvo::Archiver::~Archiver() = default;

void Kryvo::Archiver::compress(const std::size_t id,
                               const QString& inputFilePath,
                               const QString& outputFilePath) {
  Q_D(Archiver);

  d->compress(id, inputFilePath, outputFilePath);
}

void Kryvo::Archiver::decompress(std::size_t id,
                                 const QString& inputFilePath,
                                 const QString& outputFilePath) {
  Q_D(Archiver);

  d->decompress(id, inputFilePath, outputFilePath);
}

void Kryvo::Archiver::archive(const QStringList& filePaths) {

}

void Kryvo::Archiver::extract(const QString& archiveFilePath) {

}
