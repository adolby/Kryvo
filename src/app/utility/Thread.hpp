#ifndef KRYVO_UTILITY_THREAD_HPP_
#define KRYVO_UTILITY_THREAD_HPP_

#include <QThread>
#include <QAbstractEventDispatcher>
#include <QVariant>
#include <QEvent>
#include <QCoreApplication>
#include <QTimer>
#include <QObject>

namespace Kryvo {

class Thread final : public QThread {
   Q_OBJECT
   Q_DISABLE_COPY(Thread)

   void run() override {
      connect(QAbstractEventDispatcher::instance(this),
              &QAbstractEventDispatcher::aboutToBlock,
              this, &Thread::aboutToBlock);
      QThread::run();
   }

   QAtomicInt inDestructor;

public:
  Thread(QObject* parent = nullptr) : QThread(parent) {
  }

   /// Take an object and prevent timer resource leaks when the object is about
   /// to become threadless.
   void takeObject(QObject *obj) {
      // Work around to prevent
      // QBasicTimer::stop: Failed. Possibly trying to stop from a different thread
      static constexpr char kRegistered[] = "__ThreadRegistered";

      static constexpr char kMoved[] = "__Moved";

      if (!obj->property(kRegistered).isValid()) {
         QObject::connect(this, &Thread::finished, obj, [this, obj]{
            if (!inDestructor.load() || obj->thread() != this)
               return;
            // The object is about to become threadless
            Q_ASSERT(obj->thread() == QThread::currentThread());
            obj->setProperty(kMoved, true);
            obj->moveToThread(this->thread());
         }, Qt::DirectConnection);
         QObject::connect(this, &QObject::destroyed, obj, [obj]{
            if (!obj->thread()) {
               obj->moveToThread(QThread::currentThread());
               obj->setProperty(kRegistered, {});
            }
            else if (obj->thread() == QThread::currentThread() && obj->property(kMoved).isValid()) {
               obj->setProperty(kMoved, {});
               QCoreApplication::sendPostedEvents(obj, QEvent::MetaCall);
            }
            else if (obj->thread()->eventDispatcher())
               QTimer::singleShot(0, obj, [obj]{ obj->setProperty(kRegistered, {}); });
         }, Qt::DirectConnection);

         obj->setProperty(kRegistered, true);
      }
      obj->moveToThread(this);
   }

   ~Thread() override {
      inDestructor.store(1);
      requestInterruption();
      quit();
      wait();
   }

   Q_SIGNAL void aboutToBlock();
};

} // namespace Kryvo

#endif // KRYVO_UTILITY_THREAD_HPP_
