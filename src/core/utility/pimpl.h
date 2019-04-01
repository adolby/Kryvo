#ifndef KRYVO_UTILITY_PIMPL_H_
#define KRYVO_UTILITY_PIMPL_H_

template <typename T> static inline T *getPtrHelper(T *ptr) { return ptr; }
template <typename Wrapper> static inline typename Wrapper::pointer getPtrHelper(const Wrapper &p) { return p.get(); }

#define DECLARE_PRIVATE(Class) \
    inline Class##Private* d_func() { return reinterpret_cast<Class##Private *>(getPtrHelper(d_ptr)); } \
    inline const Class##Private* d_func() const { return reinterpret_cast<const Class##Private *>(getPtrHelper(d_ptr)); } \
    friend class Class##Private;

#endif // KRYVO_UTILITY_PIMPL_H_
