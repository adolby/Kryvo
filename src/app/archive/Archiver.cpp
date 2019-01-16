#include "archive/Archiver.hpp"
#include "Constants.hpp"
#include <QDir>
#include <QSaveFile>
#include <QFile>
#include <QStringBuilder>
#include "zlib.h"

class Kryvo::ArchiverPrivate {
  Q_DISABLE_COPY(ArchiverPrivate)
  Q_DECLARE_PUBLIC(Archiver)

 public:
  ArchiverPrivate(Archiver* archiver, DispatcherState* ds);

  int gzipDeflateFile(int id, const QString& sourceFilePath,
                      const QString& destFilePath,
                      int level = Z_DEFAULT_COMPRESSION);
  int gzipInflateFile(int id, const QString& sourceFilePath,
                      const QString& destFilePath);
  QByteArray compressChunk(const QByteArray& chunk);
  bool compressFile(int id, const QString& inFilePath,
                    const QString& outFilePath);
  bool decompressFile(int id, const QString& inFilePath,
                      const QString& outFilePath);
  void compress(int id, const QString& inFilePath,
                const QString& outputPath);
  void decompress(int id, const QString& inFilePath,
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
int Kryvo::ArchiverPrivate::gzipDeflateFile(int id,
                                            const QString& sourceFilePath,
                                            const QString& destFilePath,
                                            int level) {
  Q_Q(Archiver);

  int ret = Z_ERRNO;
  int flush = 0;
  unsigned have = 0;

  QFile source(sourceFilePath);
  const bool sourceFileOpen = source.open(QIODevice::ReadOnly);
  if (!sourceFileOpen) {
    return ret;
  }

  QSaveFile dest(destFilePath);
  const bool destFileOpen = dest.open(QIODevice::WriteOnly);
  if (!destFileOpen) {
    return ret;
  }

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

  /* compress until end of file */
  do {
    const qint64 bytesAvailable = source.read(in.data(), kChunk);

    if (bytesAvailable < 0) {
      deflateEnd(&strm);
      return Z_ERRNO;
    }

    strm.avail_in = bytesAvailable;

    flush = source.atEnd() ? Z_FINISH : Z_NO_FLUSH;

    strm.next_in = reinterpret_cast<unsigned char*>(in.data());

    /* run deflate() on input until output buffer not full, finish
       compression if all of source has been read in */
    do {
      strm.avail_out = kChunk;
      strm.next_out = reinterpret_cast<unsigned char*>(out.data());
      ret = deflate(&strm, flush); /* no bad return value */

      Q_ASSERT(ret != Z_STREAM_ERROR); /* state not clobbered */

      have = kChunk - strm.avail_out;

      const qint64 bytesWritten = dest.write(out.data(), have);

      const qint64 percentProgress =
        static_cast<qint64>(bytesWritten / bytesAvailable);

      emit q->fileProgress(id, QObject::tr("Compressing"), percentProgress);

      if (bytesWritten != have || bytesWritten < 0) {
        deflateEnd(&strm);
        return Z_ERRNO;
      }
    } while (0 == strm.avail_out);

    Q_ASSERT(0 == strm.avail_in); /* all input will be used */

    /* done when last data in file processed */
  } while (flush != Z_FINISH);

  Q_ASSERT(ret == Z_STREAM_END); /* stream will be complete */

  /* clean up and return */
  deflateEnd(&strm);

  dest.commit();

  emit q->fileProgress(id, QObject::tr("Compressed"), 100);

  return Z_OK;
}

/* Decompress from file source to file dest until stream ends or EOF.
   Returns Z_OK on success, Z_MEM_ERROR if memory could not be
   allocated for processing, Z_DATA_ERROR if the deflate data is
   invalid or incomplete, Z_VERSION_ERROR if the version of zlib.h and
   the version of the library linked do not match, or Z_ERRNO if there
   is an error reading or writing the files. */
int Kryvo::ArchiverPrivate::gzipInflateFile(int id,
                                            const QString& sourceFilePath,
                                            const QString& destFilePath) {
  Q_Q(Archiver);

  int ret = Z_ERRNO;

  if ( !state ) {
    return ret;
  }

  unsigned have = 0;

  z_stream strm;

  QFile source(sourceFilePath);
  const bool sourceFileOpen = source.open(QIODevice::ReadOnly);
  if (!sourceFileOpen) {
    return ret;
  }

  QSaveFile dest(destFilePath);
  const bool destFileOpen = dest.open(QIODevice::WriteOnly);
  if (!destFileOpen) {
    return ret;
  }

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

  /* decompress until deflate stream ends or end of file */
  do {
    const qint64 bytesAvailable = source.read(in.data(), kChunk);

    if (bytesAvailable < 0) {
      inflateEnd(&strm);
      return Z_ERRNO;
    }

    strm.avail_in = bytesAvailable;

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
        return ret;
      }

      have = kChunk - strm.avail_out;

      const qint64 bytesWritten = dest.write(out.data(), have);

      const qint64 percentProgress =
        static_cast<qint64>(bytesWritten / bytesAvailable);

      emit q->fileProgress(id, QObject::tr("Decompressing"), percentProgress);

      if (bytesWritten != have || bytesWritten < 0) {
        inflateEnd(&strm);
        return Z_ERRNO;
      }
    } while (0 == strm.avail_out);

    /* done when inflate() says it's done */
  } while (ret != Z_STREAM_END);

  /* clean up and return */
  inflateEnd(&strm);

  dest.commit();

  emit q->fileProgress(id, QObject::tr("Decompressed"), 100);

  return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}

QByteArray Kryvo::ArchiverPrivate::compressChunk(const QByteArray& chunk) {
    return QByteArray();
}

bool Kryvo::ArchiverPrivate::compressFile(int id,
                                          const QString& inputFilePath,
                                          const QString& outputFilePath) {
  Q_Q(Archiver);

  const QFileInfo inputFileInfo(inputFilePath);

  if (!inputFileInfo.exists() || !inputFileInfo.isFile() ||
      !inputFileInfo.isReadable()) {
    emit q->errorMessage(Constants::messages[11], inputFilePath);
    return false;
  }

  const int ret = gzipDeflateFile(id, inputFilePath, outputFilePath,
                                  Z_DEFAULT_COMPRESSION);

  if (ret != Z_OK) {
    emit q->errorMessage(Constants::messages[11], inputFilePath);
    return false;
  }

  emit q->fileCompressed(id, outputFilePath);

  return true;
}

bool Kryvo::ArchiverPrivate::decompressFile(int id,
                                            const QString& inputFilePath,
                                            const QString& outputFilePath) {
  Q_Q(Archiver);

  const QFileInfo inputFileInfo(inputFilePath);

  if (!inputFileInfo.exists() || !inputFileInfo.isFile() ||
      !inputFileInfo.isReadable()) {
    emit q->errorMessage(Constants::messages[12], inputFilePath);
    return false;
  }

  const int ret = gzipInflateFile(id, inputFilePath, outputFilePath);

  if (ret != Z_OK) {
    emit q->errorMessage(Constants::messages[12], inputFilePath);
  }

  emit q->fileDecompressed(id, outputFilePath);

  return true;
}

void Kryvo::ArchiverPrivate::compress(int id,
                                      const QString& inputFilePath,
                                      const QString& outputFilePath) {
  compressFile(id, inputFilePath, outputFilePath);
}

void Kryvo::ArchiverPrivate::decompress(int id,
                                        const QString& inputFilePath,
                                        const QString& outputFilePath) {
  decompressFile(id, inputFilePath, outputFilePath);
}

Kryvo::Archiver::Archiver(DispatcherState* state, QObject* parent)
  : QObject(parent), d_ptr(std::make_unique<ArchiverPrivate>(this, state)) {
}

Kryvo::Archiver::~Archiver() = default;

void Kryvo::Archiver::compress(int id, const QString& inputFilePath,
                               const QString& outputFilePath) {
  Q_D(Archiver);

  d->compress(id, inputFilePath, outputFilePath);
}

void Kryvo::Archiver::decompress(int id,
                                 const QString& inputFilePath,
                                 const QString& outputFilePath) {
  Q_D(Archiver);

  d->decompress(id, inputFilePath, outputFilePath);
}

void Kryvo::Archiver::archive(const QStringList& filePaths) {

}

void Kryvo::Archiver::extract(const QString& archiveFilePath) {

}
