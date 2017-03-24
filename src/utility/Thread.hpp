#ifndef KRYVOS_UTILITY_THREAD_HPP_
#define KRYVOS_UTILITY_THREAD_HPP_

#include <QThread>
#include <QObject>

namespace Kryvos {

class Thread : public QThread {
  Q_OBJECT
  using QThread::run; // This is a final class

 public:
  explicit Thread(QObject* parent = Q_NULLPTR);
  ~Thread();
};

}

#endif // KRYVOS_UTILITY_THREAD_HPP_
