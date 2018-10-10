#include "archive/Archiver.hpp"
#include <QDir>
#include <QFile>
#include <QStringBuilder>
#include "zlib.h"

#include <QDebug>

class Kryvo::ArchiverPrivate {
  Q_DISABLE_COPY(ArchiverPrivate)

 public:
  ArchiverPrivate();

  int def(const QString& sourceFilePath, const QString& destFilePath,
          int level = Z_DEFAULT_COMPRESSION);
  int inf(const QString& sourceFilePath, const QString& destFilePath);

  static constexpr qint64 kChunk = 16384;
};

Kryvo::ArchiverPrivate::ArchiverPrivate() = default;

/* Compress from file source to file dest until EOF on source.
   def() returns Z_OK on success, Z_MEM_ERROR if memory could not be
   allocated for processing, Z_STREAM_ERROR if an invalid compression
   level is supplied, Z_VERSION_ERROR if the version of zlib.h and the
   version of the library linked do not match, or Z_ERRNO if there is
   an error reading or writing the files. */
int Kryvo::ArchiverPrivate::def(const QString& sourceFilePath,
                                const QString& destFilePath, int level) {
  int ret = Z_ERRNO;
  int flush = 0;
  unsigned have = 0;

  QFile source(sourceFilePath);
  const bool sourceFileOpen = source.open(QIODevice::ReadOnly);
  if (!sourceFileOpen) {
      return ret;
  }

  QFile dest(destFilePath);
  const bool destFileOpen = dest.open(QIODevice::WriteOnly);
  if (!destFileOpen) {
      return ret;
  }

  unsigned char in[kChunk];
  unsigned char out[kChunk];

  /* allocate deflate state */
  z_stream strm;
  strm.zalloc = Z_NULL;
  strm.zfree = Z_NULL;
  strm.opaque = Z_NULL;

  ret = deflateInit(&strm, level);

  if (ret != Z_OK) {
    return ret;
  }

  /* compress until end of file */
  do {
    const int bytesAvailable =
      source.read(reinterpret_cast<char*>(in), kChunk);

    if (bytesAvailable < 0) {
      deflateEnd(&strm);
      return Z_ERRNO;
    }

    strm.avail_in = bytesAvailable;

    flush = source.atEnd() ? Z_FINISH : Z_NO_FLUSH;

    strm.next_in = in;

    /* run deflate() on input until output buffer not full, finish
       compression if all of source has been read in */
    do {
      strm.avail_out = kChunk;
      strm.next_out = out;
      ret = deflate(&strm, flush);    /* no bad return value */

      Q_ASSERT(ret != Z_STREAM_ERROR);  /* state not clobbered */

      have = kChunk - strm.avail_out;

      const qint64 bytesWritten = dest.write(reinterpret_cast<char*>(out),
                                             have);

      if (bytesWritten != have || bytesWritten < 0) {
        deflateEnd(&strm);
        return Z_ERRNO;
      }
    } while (0 == strm.avail_out);

    Q_ASSERT(0 == strm.avail_in);     /* all input will be used */

    /* done when last data in file processed */
  } while (flush != Z_FINISH);

  Q_ASSERT(ret == Z_STREAM_END);        /* stream will be complete */

  /* clean up and return */
  deflateEnd(&strm);

  return Z_OK;
}

/* Decompress from file source to file dest until stream ends or EOF.
   inf() returns Z_OK on success, Z_MEM_ERROR if memory could not be
   allocated for processing, Z_DATA_ERROR if the deflate data is
   invalid or incomplete, Z_VERSION_ERROR if the version of zlib.h and
   the version of the library linked do not match, or Z_ERRNO if there
   is an error reading or writing the files. */
int Kryvo::ArchiverPrivate::inf(const QString& sourceFilePath,
                                const QString& destFilePath) {
  int ret = Z_ERRNO;
  unsigned have = 0;

  z_stream strm;

  QFile source(sourceFilePath);
  const bool sourceFileOpen = source.open(QIODevice::ReadOnly);
  if (!sourceFileOpen) {
      return ret;
  }

  QFile dest(destFilePath);
  const bool destFileOpen = dest.open(QIODevice::WriteOnly);
  if (!destFileOpen) {
      return ret;
  }

  unsigned char in[kChunk];
  unsigned char out[kChunk];

  /* allocate inflate state */
  strm.zalloc = Z_NULL;
  strm.zfree = Z_NULL;
  strm.opaque = Z_NULL;
  strm.avail_in = 0;
  strm.next_in = Z_NULL;

  ret = inflateInit(&strm);

  if (ret != Z_OK) {
    return ret;
  }

  /* decompress until deflate stream ends or end of file */
  do {
    const int bytesAvailable = source.read(reinterpret_cast<char*>(in), kChunk);

    if (bytesAvailable < 0) {
      inflateEnd(&strm);
      return Z_ERRNO;
    }

    strm.avail_in = bytesAvailable;

    if (0 == strm.avail_in) {
      break;
    }

    strm.next_in = in;

    /* run inflate() on input until output buffer not full */
    do {
      strm.avail_out = kChunk;
      strm.next_out = out;
      ret = inflate(&strm, Z_NO_FLUSH);

      Q_ASSERT(ret != Z_STREAM_ERROR);  /* state not clobbered */

      switch (ret) {
      case Z_NEED_DICT:
        ret = Z_DATA_ERROR;     /* and fall through */
      case Z_DATA_ERROR:
      case Z_MEM_ERROR:
        inflateEnd(&strm);
        return ret;
      }

      have = kChunk - strm.avail_out;

      const qint64 bytesWritten = dest.write(reinterpret_cast<char*>(out),
                                             have);

      if (bytesWritten != have || bytesWritten < 0) {
        inflateEnd(&strm);
        return Z_ERRNO;
      }
    } while (0 == strm.avail_out);

    /* done when inflate() says it's done */
  } while (ret != Z_STREAM_END);

  /* clean up and return */
  inflateEnd(&strm);

  return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}

/* report a zlib or i/o error */
//QString error(int ret) {
//    switch (ret) {
//    case Z_ERRNO:
//        if (ferror(stdin))
//            fputs("Error reading stdin\n", stderr);
//        if (ferror(stdout))
//            fputs("Error writing stdout\n", stderr);
//        break;
//    case Z_STREAM_ERROR:
//        fputs("invalid compression level\n", stderr);
//        break;
//    case Z_DATA_ERROR:
//        fputs("invalid or incomplete deflate data\n", stderr);
//        break;
//    case Z_MEM_ERROR:
//        fputs("out of memory\n", stderr);
//        break;
//    case Z_VERSION_ERROR:
//        fputs("zlib version mismatch!\n", stderr);
//    }
//}

Kryvo::Archiver::Archiver(QObject* parent)
  : QObject(parent) {
}

Kryvo::Archiver::~Archiver() = default;

void Kryvo::Archiver::compressFile(const QString& inFilePath,
                                   const QString& outFilePath) {
  Q_D(Archiver);

  if (inFilePath.isEmpty()) {
    // TODO Unify app error strings
    emit errorMessage(QStringLiteral("No input path provided for compression"));
    return;
  }

  const QString& filePath = outFilePath.isEmpty() ? inFilePath : outFilePath;

  const int ret = d->inf(filePath, outFilePath);

  if (ret != Z_OK) {
    // TODO Unify app error strings
    emit errorMessage(QStringLiteral("File compression error"));
  }
}

void Kryvo::Archiver::decompressFile(const QString& inFilePath,
                                     const QString& outFilePath) {
  Q_D(Archiver);

  if (inFilePath.isEmpty()) {
    // TODO Unify app error strings
    emit errorMessage(QStringLiteral("No input path provided for "
                                     "decompression"));
    return;
  }

  const QFileInfo inFileInfo(inFilePath);
  const QString& inPath = inFileInfo.absolutePath();

  const QString& filePath = outFilePath.isEmpty() ? inPath : outFilePath;

  const int ret = d->def(filePath, outFilePath, Z_DEFAULT_COMPRESSION);

  if (ret != Z_OK) {
    // TODO Unify app error strings
    emit errorMessage(QStringLiteral("File decompression error"));
  }
}

void Kryvo::Archiver::archive(const QStringList& inFilePaths) {

}

void Kryvo::Archiver::extract(const QString& inFilePath) {

}
