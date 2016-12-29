#ifndef KRYVOS_CRYPTOGRAPHY_ARCHIVE_H_
#define KRYVOS_CRYPTOGRAPHY_ARCHIVE_H_

#include <QStringList>
#include <QString>
#include <archive.h>
#include <archive_entry.h>

namespace Kryvos {

inline namespace Cryptography {

inline QStringList list_archive_files(const QString& filePath) {
  struct archive* a;
  struct archive_entry* entry;

  a = archive_read_new();
  archive_read_support_filter_all(a);
  archive_read_support_format_all(a);

  archive_read_open_filename(a, filePath.toStdString().c_str(), 10240);

  QStringList fileList;

  while (ARCHIVE_OK == archive_read_next_header(a, &entry)) {
    fileList << QString{archive_entry_pathname(entry)};
    archive_read_data_skip(a);
  }

  archive_read_free(a);

  return fileList;
}

inline int copy_data(struct archive* ar, struct archive* aw) {
  int statusCode = 0;

  const void* buff;
  size_t size;
  off_t offset;

  while (true) {
    const auto readCode = archive_read_data_block(ar, &buff, &size, &offset);

    if (ARCHIVE_EOF == readCode) {
      statusCode = ARCHIVE_OK;
      break;
    }

    if (readCode < ARCHIVE_OK) {
      statusCode = readCode;
      break;
    }

    const auto writeCode = archive_write_data_block(aw, buff, size, offset);

    if (writeCode < ARCHIVE_OK) {
      statusCode = writeCode;
      break;
    }
  }

  return statusCode;
}

inline int extract(const QString& archivePath, const QString& outputPath) {
  struct archive* a;
  struct archive* ext;
  struct archive_entry* entry;

  auto flags = ARCHIVE_EXTRACT_TIME;
  flags |= ARCHIVE_EXTRACT_PERM;
  flags |= ARCHIVE_EXTRACT_ACL;
  flags |= ARCHIVE_EXTRACT_FFLAGS;

  a = archive_read_new();
  archive_read_support_format_all(a);
  archive_read_support_compression_all(a);

  ext = archive_write_disk_new();
  archive_write_disk_set_options(ext, flags);
  archive_write_disk_set_standard_lookup(ext);

  const auto readOpenCode =
    archive_read_open_filename(a, archivePath.toStdString().c_str(), 10240);

  if (readOpenCode) {
    return 1;
  }

  while (true) {
    const auto readNextHeaderCode = archive_read_next_header(a, &entry);

    if (ARCHIVE_EOF == readNextHeaderCode) {
      break;
    }
    else if (readNextHeaderCode < ARCHIVE_WARN) {
      return 1;
    }

    const char* currentFile = archive_entry_pathname(entry);
    const auto fullOutputPath = std::string{outputPath.toStdString() +
                                            currentFile};
    archive_entry_set_pathname(entry, fullOutputPath.c_str());

    archive_write_header(ext, entry);

    const auto entrySize = archive_entry_size(entry);

    if (entrySize > 0) {
      const auto copyDataCode = copy_data(a, ext);

      if (copyDataCode < ARCHIVE_WARN) {
        return 1;
      }
    }

    const auto writeFinishCode = archive_write_finish_entry(ext);

    if (writeFinishCode < ARCHIVE_WARN) {
      return 1;
    }
  }

  archive_read_close(a);
  archive_read_free(a);
  archive_write_close(ext);
  archive_write_free(ext);

  return 0;
}

}

}

#endif // KRYVOS_CRYPTOGRAPHY_ARCHIVE_H_
