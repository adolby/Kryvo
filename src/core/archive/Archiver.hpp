#ifndef KRYVO_ARCHIVE_ARCHIVER_HPP_
#define KRYVO_ARCHIVE_ARCHIVER_HPP_

#include "Pipe.hpp"
#include "SchedulerState.hpp"
#include "utility/pimpl.h"
#include <QObject>
#include <QFileInfo>
#include <vector>
#include <memory>

namespace Kryvo {

class ArchiverPrivate;

class Archiver : public Pipe {
  Q_OBJECT
  Q_DISABLE_COPY(Archiver)
  DECLARE_PRIVATE(Archiver)
  std::unique_ptr<ArchiverPrivate> const d_ptr;

 public:
  explicit Archiver(SchedulerState* state, QObject* parent = nullptr);

  ~Archiver() override;

  bool compressFile(std::size_t id, const QFileInfo& inputFileInfo,
                    const QFileInfo& outputFileInfo);

  bool decompressFile(std::size_t id, const QFileInfo& inputFileInfo,
                      const QFileInfo& outputFileInfo);

 public slots:
  void compress(std::size_t id, const QFileInfo& inputFileInfo,
                const QFileInfo& outputFileInfo);

  void decompress(std::size_t id, const QFileInfo& inputFileInfo,
                  const QFileInfo& outputFileInfo);

//  void archive(const std::vector<QFileInfo>& inputFiles);

//  void extract(const QFileInfo& archiveFileInfo);
};

} // namespace Kryvo

#endif // KRYVO_ARCHIVE_ARCHIVER_HPP_
