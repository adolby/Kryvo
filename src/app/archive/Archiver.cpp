#include "archive/Archiver.hpp"
#include "Constants.hpp"
#include <QDir>
#include <QFile>
#include <QStringBuilder>
#include "zlib.h"

#include <QDebug>

class Kryvo::ArchiverPrivate {
  Q_DISABLE_COPY(ArchiverPrivate)

 public:
  ArchiverPrivate(Archiver* q);

  int gzipDeflateFile(const QString& sourceFilePath,
                      const QString& destFilePath,
                      int level = Z_DEFAULT_COMPRESSION);
  int gzipInflateFile(const QString& sourceFilePath,
                      const QString& destFilePath);

  static constexpr qint64 kChunk = 16384;

  Archiver* q_ptr = nullptr;
};

Kryvo::ArchiverPrivate::ArchiverPrivate(Archiver* q)
  : q_ptr(q) {
}

/* Compress from file source to file dest until EOF on source.
   def() returns Z_OK on success, Z_MEM_ERROR if memory could not be
   allocated for processing, Z_STREAM_ERROR if an invalid compression
   level is supplied, Z_VERSION_ERROR if the version of zlib.h and the
   version of the library linked do not match, or Z_ERRNO if there is
   an error reading or writing the files. */
int Kryvo::ArchiverPrivate::gzipDeflateFile(const QString& sourceFilePath,
                                            const QString& destFilePath,
                                            int level) {
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

  ret = deflateInit2(&strm, level, Z_DEFLATED, MAX_WBITS + 16, 8,
                     Z_DEFAULT_STRATEGY);

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
      ret = deflate(&strm, flush); /* no bad return value */

      Q_ASSERT(ret != Z_STREAM_ERROR); /* state not clobbered */

      have = kChunk - strm.avail_out;

      const qint64 bytesWritten = dest.write(reinterpret_cast<char*>(out),
                                             have);

      const qint64 percentProgress =
        static_cast<qint64>(bytesWritten / bytesAvailable);

      q_ptr->fileProgress(sourceFilePath, QObject::tr("Compression"),
                          percentProgress);

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

  q_ptr->fileProgress(sourceFilePath, QObject::tr("Compression"),
                      100);

  return Z_OK;
}

/* Decompress from file source to file dest until stream ends or EOF.
   inf() returns Z_OK on success, Z_MEM_ERROR if memory could not be
   allocated for processing, Z_DATA_ERROR if the deflate data is
   invalid or incomplete, Z_VERSION_ERROR if the version of zlib.h and
   the version of the library linked do not match, or Z_ERRNO if there
   is an error reading or writing the files. */
int Kryvo::ArchiverPrivate::gzipInf(const QString& sourceFilePath,
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

  ret = inflateInit2(&strm, MAX_WBITS + 16);

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

      const qint64 bytesWritten = dest.write(reinterpret_cast<char*>(out),
                                             have);

      const qint64 percentProgress =
        static_cast<qint64>(bytesWritten / bytesAvailable);

      q_ptr->fileProgress(sourceFilePath, QObject::tr("Decompression"),
                          percentProgress);

      if (bytesWritten != have || bytesWritten < 0) {
        inflateEnd(&strm);
        return Z_ERRNO;
      }
    } while (0 == strm.avail_out);

    /* done when inflate() says it's done */
  } while (ret != Z_STREAM_END);

  /* clean up and return */
  inflateEnd(&strm);

  q_ptr->fileProgress(sourceFilePath, QObject::tr("Decompression"),
                      100);

  return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}

Kryvo::Archiver::Archiver(QObject* parent)
  : QObject(parent), d_ptr(std::make_unique<ArchiverPrivate>(this)) {
}

Kryvo::Archiver::~Archiver() = default;

QByteArray compressChunk(const QByteArray& chunk) {
    return
}

void Kryvo::Archiver::compressFiles(const QStringList& inputFilePaths,
                                    const QString& outputPath) {
  const QDir outputDir(outputPath);

  // Create output path if it doesn't exist
  if (!outputDir.exists()) {
    outputDir.mkpath(outputPath);
  }

  for (const QString& inputFilePath : inputFilePaths) {
    const QFileInfo inputFileInfo(inputFilePath);
    const QString& inFilePath = inputFileInfo.absoluteFilePath();

    const QString& outPath = outputDir.exists() ?
                             outputDir.absolutePath() :
                             inputFileInfo.absolutePath();

    const QString& outFilePath =
      QString(outPath % QStringLiteral("/") % inputFileInfo.fileName() %
              Kryvo::Constants::kDot %
              Kryvo::Constants::kCompressedFileExtension);

    compressFile(inFilePath, outFilePath);
  }
}

void Kryvo::Archiver::decompressFiles(const QStringList& inputFilePaths,
                                      const QString& outputPath) {
  const QDir outputDir(outputPath);

  // Create output path if it doesn't exist
  if (!outputDir.exists()) {
    outputDir.mkpath(outputPath);
  }

  for (const QString& inputFilePath : inputFilePaths) {
    const QFileInfo inputFileInfo(inputFilePath);
    const QString& outPath = outputDir.exists() ?
                             outputDir.absolutePath() :
                             inputFileInfo.absolutePath();

    const QString& inFilePath = inputFileInfo.absoluteFilePath();

    const QString& outputFilePath = QString(outPath % QStringLiteral("/") %
                                            inputFileInfo.fileName());

    // Remove the gz extension if at the end of the file path
    const QString& choppedOutputFilePath =
      Constants::removeExtension(outputFilePath,
                                 Constants::kEncryptedFileExtension);

    // Create a unique file name for the file in this directory
    const QString& uniqueOutputFilePath =
      Constants::uniqueFilePath(choppedOutputFilePath);

    decompressFile(inFilePath, uniqueOutputFilePath);
  }
}

bool Kryvo::Archiver::compressFile(const QString& inputFilePath,
                                   const QString& outputFilePath) {
  Q_D(Archiver);

  const QFileInfo inputFileInfo(inputFilePath);

  if (!inputFileInfo.exists() || !inputFileInfo.isFile() ||
      !inputFileInfo.isReadable()) {
    emit errorMessage(Constants::messages[11], inputFilePath);
    return false;
  }

  const int ret = d->gzipDef(inputFilePath, outputFilePath,
                             Z_DEFAULT_COMPRESSION);

  if (ret != Z_OK) {
    emit errorMessage(Constants::messages[11], inputFilePath);
    return false;
  }

  return true;
}

bool Kryvo::Archiver::decompressFile(const QString& inputFilePath,
                                     const QString& outputFilePath) {
  Q_D(Archiver);

  const QFileInfo inputFileInfo(inputFilePath);

  if (!inputFileInfo.exists() || !inputFileInfo.isFile() ||
      !inputFileInfo.isReadable()) {
    emit errorMessage(Constants::messages[12], inputFilePath);
    return false;
  }

  const int ret = d->gzipInf(inputFilePath, outputFilePath);

  if (ret != Z_OK) {
    emit errorMessage(Constants::messages[12], inputFilePath);
  }

  return true;
}

void Kryvo::Archiver::archive(const QStringList& filePaths) {

}

void Kryvo::Archiver::extract(const QString& archiveFilePath) {

}
