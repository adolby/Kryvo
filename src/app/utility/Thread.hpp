#ifndef KRYVO_UTILITY_THREAD_HPP_
#define KRYVO_UTILITY_THREAD_HPP_

#include <QThread>
#include <QObject>

namespace Kryvo {

class Thread : public QThread {
  Q_OBJECT
  using QThread::run; // This is a final class

 public:
  explicit Thread(QObject* parent = nullptr);
  ~Thread();
};

}

#endif // KRYVO_UTILITY_THREAD_HPP_
